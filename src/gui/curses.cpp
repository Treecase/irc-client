/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  UI for the IRC client
 */

#include "../commands.h"
#include "../data.h"
#include "../logging.h"
#include "../socket.h"

#include <curses.h>
#include <panel.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>

#include <deque>
#include <functional>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>



struct Channel
{
    std::set<std::string> users;
    size_t user_list_idx;
    size_t channel_list_idx;
    size_t chat_log_idx;
    std::deque<std::string> chat_log;
};



static std::deque<std::string> send_queue{};
static std::deque<std::string> recv_queue{};

static std::string current_channel = "";
static std::string *G_username = nullptr;

static std::unordered_map<std::string, struct Channel> channels{};

static WINDOW *chatw = nullptr;
static WINDOW *inputw = nullptr;
static WINDOW *userw = nullptr;
static WINDOW *channelsw = nullptr;

static bool resize_event = false,
            userw_hidden = true;


static std::string str_upper(std::string str);


static char const PROMPT[] = "> ";

static std::vector<std::string> UI_COMMANDS_HELP =
{
    "----- Begin Help -----",
    "Once you've joined a channel, you can see a list",
    "of active users by right-clicking.",
    "",
    "Commands:",
    "/HELP - Get help.",
    "/JOIN <channel> - Join a channel.",
    "/PART [channel] - Leave current/given channel.",
    "/QUIT [message] - Quit.",
    "/QUOTE <command> - Pass a literal IRC command.",
    "",
    "For more help, try '/QUOTE HELP'",
    "----- End of Help -----"
};

static std::unordered_map<
    std::string,
    std::function<std::string(std::string)>> UI_COMMANDS =
{
    {   "HELP",
        [](std::string str)
        {
            for (auto &help_string : UI_COMMANDS_HELP)
            {
                recv_queue.push_back("NOTICE :help: " + help_string);
            }
            return "";
        }
    },
    {   "JOIN",
        [](std::string str)
        {
            current_channel = str_upper(str);
            return "JOIN " + str;
        }
    },
    {   "PART",
        [](std::string str)
        {
            if (str.empty())
            {
                str = current_channel;
            }
            return "PART " + str_upper(str);
        }
    },
    {   "QUIT",
        [](std::string str)
        {
            return "QUIT :" + str;
        }
    },
    {   "QUOTE",
        [](std::string str)
        {
            return str;
        }
    },
};



template<typename T, typename K>
static bool contains(T container, K key)
{
    try
    {
        container.at(key);
    }
    catch (std::out_of_range &e)
    {
        return false;
    }
    return true;
}

static std::string str_upper(std::string str)
{
    std::string newstr = str;
    for (char &ch : newstr)
    {
        /* in IRC, these symbols have uppercase versions */
        switch (ch)
        {
        case '{':
            ch = '[';
            break;
        case '}':
            ch = ']';
            break;
        case '|':
            ch = '\\';
            break;
        case '^':
            ch = '~';
            break;

        default:
            ch = toupper(ch);
            break;
        }
    }
    return newstr;
}

void input_handler(std::string line)
{
    if (!line.empty())
    {
        /* lines starting with a '/' are commands,
         * so just strip the slash and send the command */
        if (line[0] == '/')
        {
            auto cmd = str_upper(line.substr(1, line.find(' ') - 1));
            if (contains(UI_COMMANDS, cmd))
            {
                auto idx = line.find(' ');
                if (idx != std::string::npos)
                {
                    send_queue.push_back(
                        UI_COMMANDS[cmd](line.substr(idx + 1)));
                }
                else
                {
                    send_queue.push_back(UI_COMMANDS[cmd](""));
                }
            }
            else
            {
                error("unknown command '%s'", cmd.c_str());
            }
        }
        /* if the line isn't a command, send it as a message */
        else
        {
            send_queue.push_back(
                "PRIVMSG "
                + current_channel
                + " :"
                + line);
            recv_queue.push_back(
                "PRIVATE "
                + *G_username
                + " :"
                + line);
        }
    }
}

void sigwinch_handler(int sig)
{
    resize_event = true;
}

void curses_cleanup(void)
{
    endwin();
}



void gui(
    int fd,
    std::string username,
    std::string password,
    std::string realname)
{
    G_username = &username;

    struct sigaction act{};
    act.sa_handler = sigwinch_handler;
    act.sa_flags = 0;
    sigaction(SIGWINCH, &act, nullptr);

    /* initialize ncurses */
    atexit(curses_cleanup);
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, nullptr);

    short colours[16] =
    {
        COLOR_WHITE,   /* white */
        COLOR_BLACK,   /* black */
        COLOR_BLUE,    /* blue */
        COLOR_GREEN,   /* green */
        COLOR_RED,     /* light red */
        COLOR_RED,     /* brown */
        COLOR_MAGENTA, /* purple */
        COLOR_RED,     /* orange */
        COLOR_YELLOW,  /* yellow */
        COLOR_GREEN,   /* light green */
        COLOR_CYAN,    /* cyan */
        COLOR_CYAN,    /* light cyan */
        COLOR_BLUE,    /* light blue */
        COLOR_RED,     /* pink */
        COLOR_WHITE,   /* grey */
        COLOR_WHITE,   /* light grey */
    };
    for (size_t fg = 0; fg < 16; ++fg)
    {
        for (size_t bg = 0; bg < 16; ++bg)
        {
            init_pair(
                (bg * 16) + fg,
                colours[fg] % 16,
                colours[bg] % 16);
        }
    }

    int channelsw_cols = std::min(20, COLS / 4);

    chatw = newwin(LINES - 1, 0, 0, channelsw_cols);
    userw = nullptr;
    channelsw = newwin(LINES - 1, channelsw_cols, 0, 0);
    inputw = newwin(1, 0, LINES-1, 0);

    scrollok(chatw, TRUE);
    scrollok(userw, TRUE);
    scrollok(channelsw, TRUE);
    intrflush(inputw, FALSE);
    keypad(inputw, TRUE);


    QueuedSocket client_socket{fd};

    send_queue.push_back("CAP LS");
    send_queue.push_back("CAP END");
    send_queue.push_back("PASS " + password);
    send_queue.push_back("NICK " + username);
    send_queue.push_back("USER " + username + " 0 * :" + realname);

    std::string input_line = "";
    for (bool running = true; running && !client_socket.is_closed; )
    {
        /* receive data from the IRC client */
        for (Message message : recv_queue)
        {
            std::string msg = "";
            if (message.command == "NUMERIC")
            {
                auto it = message.params.cbegin();
                char *endptr = nullptr;
                long num = strtol(it->c_str(), &endptr, 10);
                if (endptr == it->c_str())
                {
                    debug("bad numeric: %s", std::string{message}.c_str());
                }
                it++;

                switch (num)
                {
                case RPL_NAMREPLY:
                  {
                    it++;
                    std::string channel = str_upper(*it++);
                    for (; it != message.params.cend(); ++it)
                    {
                        channels[channel].users.insert(*it);
                    }
                  } break;

                case RPL_ENDOFNAMES:
                    break;

                default:
                    msg = "[" + std::to_string(num) + "]:";
                    for (; it != message.params.cend(); ++it)
                    {
                        msg += " " + *it;
                    }
                    break;
                }
            }
            else if (message.command == "NOTICE")
            {
                msg = message.params.back();
            }
            else if (message.command == "JOIN")
            {
                auto channel = str_upper(message.params.back());
                channels[channel].users.insert(message.params[0]);
                msg =\
                    message.params[0]
                    + " ("
                    + message.params[1]
                    + ") joined";
            }
            else if (message.command == "PART")
            {
                auto channel = str_upper(message.params.back());
                auto user = message.params[0];
                if (contains(channels, channel))
                {
                    if (user == *G_username)
                    {
                        channels.erase(channel);
                        if (channel == current_channel)
                        {
                            if (channels.size() != 0)
                            {
                                current_channel =\
                                    channels.cbegin()->first;
                            }
                            else
                            {
                                current_channel = "";
                            }
                        }
                    }
                    else
                    {
                        channels[channel].users.erase(user);
                        msg = user + " left";
                    }
                }
            }
            else if (message.command == "QUIT")
            {
                for (auto &channel : channels)
                {
                    channel.second.users.erase(message.params[0]);
                }
                msg =\
                    message.params[0]
                    + " quit ("
                    + message.params.back()
                    + ")";
            }
            else if (message.command == "PRIVATE")
            {
                msg =\
                    message.params[0]
                    + ": "
                    + message.params.back();
            }
            else if (message.command == "ERROR")
            {
                msg = "ERROR: " + message.params.back();
            }
            else if (message.command == "EXIT")
            {
                running = false;
                break;
            }
            else
            {
                msg = "??? [" + message.command;
                for (auto p : message.params)
                {
                    msg += " " + p;
                }
                msg += "]";
            }

            if (!msg.empty())
            {
                channels[current_channel].chat_log.push_front(msg);
            }
        }
        recv_queue.clear();
        if (!running)
        {
            break;
        }

        /* terminal window resized */
        if (resize_event)
        {
            endwin();
            refresh();

            delwin(chatw);
            delwin(channelsw);
            delwin(inputw);
            if (!userw_hidden)
            {
                delwin(userw);
            }

            int userw_cols = userw_hidden? 0 : std::min(20, COLS / 4);
            int channelsw_cols = std::min(20, COLS / 4);

            chatw = newwin(LINES - 1, COLS - userw_cols, 0, channelsw_cols);
            channelsw = newwin(LINES - 1, channelsw_cols, 0, 0);
            inputw = newwin(1, COLS - userw_cols, LINES - 1, 0);
            if (!userw_hidden)
            {
                userw = newwin(0, userw_cols, 0, COLS - userw_cols-1);
                scrollok(userw, TRUE);
            }

            scrollok(chatw, TRUE);
            scrollok(channelsw, TRUE);
            intrflush(inputw, FALSE);
            keypad(inputw, TRUE);

            resize_event = false;
        }
        /* redraw the screen, since the user window was hidden */
        if (userw_hidden && userw != nullptr)
        {
            delwin(chatw);
            delwin(userw);
            delwin(channelsw);
            delwin(inputw);

            chatw = newwin(LINES - 1, 0, 0, channelsw_cols);
            userw = nullptr;
            channelsw = newwin(LINES - 1, channelsw_cols, 0, 0);
            inputw = newwin(1, 0, LINES-1, 0);

            scrollok(chatw, TRUE);
            scrollok(channelsw, TRUE);
            intrflush(inputw, FALSE);
            keypad(inputw, TRUE);
        }
        /* redraw the screen, since the user window was unhidden */
        if (!userw_hidden && userw == nullptr)
        {
            delwin(chatw);
            delwin(channelsw);
            delwin(inputw);

            int userw_cols = userw_hidden? 0 : std::min(20, COLS / 4);

            chatw = newwin(
                LINES - 1, COLS - userw_cols - channelsw_cols,
                0, channelsw_cols);
            channelsw = newwin(LINES - 1, channelsw_cols, 0, 0);
            userw = newwin(0, userw_cols, 0, COLS - userw_cols - 1);
            inputw = newwin(1, COLS - userw_cols, LINES - 1, 0);

            scrollok(chatw, TRUE);
            scrollok(channelsw, TRUE);
            scrollok(userw, TRUE);
            intrflush(inputw, FALSE);
            keypad(inputw, TRUE);
        }

        /* update the chat window */
        werase(chatw);
        wmove(chatw, getmaxy(chatw) - 1, 0);
        bool done = false;
        for (
            size_t i = channels[current_channel].chat_log_idx;
            i < channels[current_channel].chat_log.size() && !done;
            ++i)
        {
            std::string fullmsg =\
                channels[current_channel].chat_log[i];

            bool bold = false,
                 italic = false,
                 reverse = false,
                 underline = false;
            char fg[3] = {0,0,0},
                 bg[3] = {0,0,0};

            for (
                ssize_t line = fullmsg.size() / getmaxx(chatw);
                line >= 0;
                --line)
            {
                if (getcury(chatw) == 0)
                {
                    done = true;
                }
                std::string msg =\
                    fullmsg.substr(
                        getmaxx(chatw) * line,
                        getmaxx(chatw));

                /* print the received messages */
                for (size_t i = 0; i < msg.size(); ++i)
                {
                    switch (msg[i])
                    {
                    case 0x02:  /* ^B */
                        bold = !bold;
                        break;

                    case 0x03:  /* ^C */
                        /* colour */
                        if (isdigit(msg[i+1]))
                        {
                            fg[0] = msg[++i];
                            if (isdigit(msg[i+1]))
                            {
                                fg[1] = msg[++i];
                            }
                            if (msg[i+1] == ',')
                            {
                                i++;
                                if (isdigit(msg[i+1]))
                                {
                                    bg[0] = msg[++i];
                                    if (isdigit(msg[i+1]))
                                    {
                                        bg[1] = msg[++i];
                                    }
                                }
                                else
                                {
                                    i--;
                                }
                            }
                        }
                        else
                        {
                            fg[0] = 0;
                            fg[1] = 0;
                            bg[0] = 0;
                            bg[1] = 0;
                        }
                        break;

                    case 0x0F:  /* ^O */
                        /* clear formatting */
                        bold = false;
                        italic = false;
                        reverse = false;
                        underline = false;
                        fg[0] = 0;
                        fg[1] = 0;
                        bg[0] = 0;
                        bg[1] = 0;
                        break;

                    case 0x16:  /* ^R */
                        reverse = !reverse;
                        break;

                    case 0x1D:  /* ^I */
                        italic = !italic;
                        break;

                    case 0x1F:  /* ^U */
                        underline = !underline;
                        break;

                    default:
                      {
                        wattr_set(chatw, A_NORMAL, 0, nullptr);
                        attr_t attrs = 0;
                        if (bold)
                        {
                            attrs |= A_BOLD;
                        }
                        if (italic)
                        {
                            attrs |= A_ITALIC;
                        }
                        if (reverse)
                        {
                            attrs |= A_REVERSE;
                        }
                        if (underline)
                        {
                            attrs |= A_UNDERLINE;
                        }
                        wattr_on(chatw, attrs, nullptr);

                        if (fg[0] != 0)
                        {
                            int fore = strtol(fg, nullptr, 10),
                                back =\
                                    (bg[0] == 0?
                                        1
                                        : strtol(bg, nullptr, 10));
                            wcolor_set(chatw, (back * 16) + fore, nullptr);
                        }
                        if (getcurx(chatw) + 1 < getmaxx(chatw))
                        {
                            waddch(chatw, msg[i]);
                        }
                      } break;
                    }
                }
                if (getcury(chatw) - 1 < 0)
                {
                    break;
                }
                wmove(chatw, getcury(chatw) - 1, 0);
            }
        }

        /* update the input window */
        werase(inputw);
        wmove(inputw, 0, 0);
        waddstr(inputw, ("> " + input_line).c_str());

        /* update the user list window */
        if (!userw_hidden)
        {
            auto it = channels[current_channel].users.cbegin();
            auto const it_end = channels[current_channel].users.cend();
            for (
                size_t i = 0;
                (   it != it_end
                    && i < channels[current_channel].user_list_idx);
                ++i)
            {
                it++;
            }
            werase(userw);
            wmove(userw, 0, 0);
            for (ssize_t i = 0; i < getmaxy(userw); ++i)
            {
                waddch(userw, ACS_VLINE);
                if (it != it_end)
                {
                    waddnstr(userw, (*it).c_str(), getmaxx(userw) - 2);
                    it++;
                }
                wmove(userw, getcury(userw) + 1, 0);
            }
        }

        /* update the channel list window */
        auto it = channels.cbegin();
        auto const it_end = channels.cend();
        for (
            size_t i = 0;
            (   it != it_end
                && i < channels[current_channel].channel_list_idx);
            ++i)
        {
            it++;
        }
        werase(channelsw);
        wmove(channelsw, 0, 0);
        for (ssize_t i = 0; i < getmaxy(channelsw); ++i)
        {
            if (it != it_end)
            {
                if (it->first == current_channel)
                {
                   wattr_on(channelsw, A_REVERSE, nullptr);
                }
                waddnstr(channelsw,
                    it->first.c_str(),
                    getmaxx(channelsw) - 2);
                it++;
               wattr_set(channelsw, A_NORMAL, 0, nullptr);
            }
            mvwaddch(channelsw,
                getcury(channelsw),
                getmaxx(channelsw) - 2,
                ACS_VLINE);
            wmove(channelsw, getcury(channelsw) + 1, 0);
        }

        /* update the screen */
        wnoutrefresh(chatw);
        wnoutrefresh(channelsw);
        wnoutrefresh(userw);
        wnoutrefresh(inputw);
        doupdate();


        /* send data to the IRC client */
        while (!send_queue.empty())
        {
            std::string str = send_queue.front();
            client_socket.write(str + "\r\n");
            send_queue.pop_front();

            debug("snd '%s'", str.c_str());
        }

        struct pollfd fds[2] = {};
        fds[0] = client_socket.get_pollfd();

        fds[1].fd = STDIN_FILENO;
        fds[1].events = POLLIN;


        int err = poll(fds, 2, -1);
        if (err == -1)
        {
            error("poll -- %s", strerror(errno));
        }
        else if (err > 0)
        {
            client_socket.update();

            /* we can read from stdin */
            if (fds[1].revents & POLLIN)
            {
                int ch = wgetch(inputw);
                switch (ch)
                {
                case ERR:
                    /* ignored */
                    break;

                case 0x04:  /* ^D */
                    if (input_line.empty())
                    {
                        debug("^D");
                        running = false;
                        break;
                    }
                    /* FALLTHROUGH */
                case '\r':
                case KEY_ENTER:
                    input_handler(input_line);
                    input_line = "";
                    break;

                case '\t':
                    /* TODO: tab completions */
                    break;

                case KEY_BACKSPACE:
                case '\b':
                    if (!input_line.empty())
                    {
                        input_line.pop_back();
                    }
                    break;

                case KEY_MOUSE:
                  {
                    MEVENT event{};
                    if (getmouse(&event) == ERR)
                    {
                        error("getmouse");
                    }
                    else
                    {
                        /* left click */
                        if (event.bstate & BUTTON1_CLICKED)
                        {
                            if (wenclose(channelsw, event.y, event.x))
                            {
                                auto it = channels.cbegin();
                                for (
                                    int i = 0;
                                    i < event.y;
                                    ++i, ++it)
                                {
                                    if (it == channels.cend())
                                    {
                                        break;
                                    }
                                }

                                if (it != channels.cend())
                                {
                                    current_channel = it->first;
                                }
                                else
                                {
                                    current_channel = "";
                                }
                            }
                        }
                        /* right click */
                        if (   event.bstate & BUTTON3_RELEASED
                            || event.bstate & BUTTON3_CLICKED)
                        {
                            userw_hidden = !userw_hidden;
                        }
                        /* scroll down */
                        if (event.bstate & BUTTON4_PRESSED)
                        {
                            if (wenclose(chatw, event.y, event.x))
                            {
                                if (  channels[current_channel].chat_log_idx + 1
                                    < channels[current_channel].chat_log.size())
                                {
                                    channels[current_channel].chat_log_idx += 1;
                                }
                            }
                            if (wenclose(channelsw, event.y, event.x))
                            {
                                if (channels[current_channel].channel_list_idx != 0)
                                {
                                    channels[current_channel].channel_list_idx -= 1;
                                }
                            }
                            if (wenclose(userw, event.y, event.x))
                            {
                                if (channels[current_channel].user_list_idx != 0)
                                {
                                    channels[current_channel].user_list_idx -= 1;
                                }
                            }
                        }
                        /* scroll up */
                        if (event.bstate & BUTTON5_PRESSED)
                        {
                            if (wenclose(chatw, event.y, event.x))
                            {
                                if (channels[current_channel].chat_log_idx != 0)
                                {
                                    channels[current_channel].chat_log_idx -= 1;
                                }
                            }
                            if (wenclose(channelsw, event.y, event.x))
                            {
                                if (  channels[current_channel].channel_list_idx + 1
                                    < channels.size())
                                {
                                    channels[current_channel].channel_list_idx += 1;
                                }
                            }
                            if (wenclose(userw, event.y, event.x))
                            {
                                if (  channels[current_channel].user_list_idx + 1
                                    < channels[
                                        current_channel
                                        ].users.size())
                                {
                                    channels[current_channel].user_list_idx += 1;
                                }
                            }
                        }
                    }
                  } break;

                default:
                    if ((unsigned int)ch < 256)
                    {
                        input_line += (char)ch;
                    }
                    break;
                }
            }
            /* we can read from the IRC client */
            if (client_socket.can_read())
            {
                std::string data = client_socket.read();

                size_t msgstart = 0;
                while (msgstart < data.size())
                {
                    size_t msgend = data.find("\r\n", msgstart);
                    if (msgend == std::string::npos)
                    {
                        break;
                    }
                    else
                    {
                        auto msg =\
                            data.substr(msgstart, msgend - msgstart);
                        debug("recv '%s'", msg.c_str());
                        recv_queue.push_back(msg);
                        msgstart = msgend + 2;
                    }
                }
            }
        }
    }

    if (!client_socket.is_closed)
    {
        client_socket.write("QUIT :\r\n");
        while (client_socket.data_to_send())
        {
            client_socket.update();
        }
    }

    close(client_socket.fd);
    curses_cleanup();
    debug("finished");
}

