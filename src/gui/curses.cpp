/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 * gui.c
 *
 *  UI for the IRC client
 *
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
};



static std::deque<std::string> send_queue{};
static std::deque<std::string> recv_queue{};

static std::string current_channel = "";
static std::string *G_username = nullptr;

static std::unordered_map<std::string, struct Channel> channels{};
static std::deque<std::string> chat_log{};
static size_t user_list_idx = 0;
static size_t chat_log_idx = 0;

static WINDOW *chatw = nullptr;
static WINDOW *inputw = nullptr;
static WINDOW *userw = nullptr;

static bool resize_event = false,
            userw_hidden = true;


static std::string str_upper(std::string str);


static char const PROMPT[] = "> ";

static std::unordered_map<
    std::string,
    std::function<std::string(std::string)>> UI_COMMANDS =
{
    {   "JOIN" ,
        [](std::string str) {
            current_channel = str_upper(str);
            return "JOIN " + str;
        }
    },
    {   "PART" ,
        [](std::string str) {
            if (str.empty())
            {
                str = current_channel;
            }
            if (channels.size() != 1)
            {
                current_channel = str_upper(channels.cbegin()->first);
            }
            else
            {
                channels.erase(current_channel);
                current_channel = "";
            }
            return "PART " + str;
        }
    },
    {   "QUIT",
        [](std::string str)
        {
            return "QUIT :" + str;
        }
    },
    { "QUOTE", [](std::string str) { return str; } },
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
    /* lines starting with a '/' are commands,
     * so just strip the slash and send the command */
    if (!line.empty())
    {
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

    chatw = newwin(LINES - 1, 0, 0, 0);
    userw = nullptr;
    inputw = newwin(1, 0, LINES-1, 0);

    scrollok(chatw, TRUE);
    scrollok(userw, TRUE);
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
                auto i = message.params.begin();
                long num = strtol((*i++).c_str(), nullptr, 10);

                switch (num)
                {
                case RPL_NAMREPLY:
                  {
                    i++;    /* '=', '*', or '@' */
                    std::string channel = str_upper(*i++);

                    for (; i != message.params.end(); ++i)
                    {
                        channels[channel].users.insert(*i);
                    }
                  } break;

                case RPL_ENDOFNAMES:
                    break;

                default:
                    for (; i != message.params.end(); ++i)
                    {
                        msg += *i;
                        if (i + 1 != message.params.end())
                        {
                            msg += " ";
                        }
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
                channels[channel].users.erase(message.params[0]);
                msg = message.params[0] + " left";
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
                chat_log.push_front(msg);
            }
        }
        recv_queue.clear();
        if (!running)
        {
            break;
        }

        if (resize_event)
        {
            endwin();
            refresh();

            delwin(chatw);
            delwin(inputw);
            if (!userw_hidden)
            {
                delwin(userw);
            }

            int userw_cols = userw_hidden? 0 : std::min(20, COLS / 4);

            chatw = newwin(LINES - 1, COLS - userw_cols, 0, 0);
            inputw = newwin(1, COLS - userw_cols, LINES - 1, 0);
            if (!userw_hidden)
            {
                userw = newwin(0, userw_cols, 0, COLS - userw_cols-1);
                scrollok(userw, TRUE);
            }

            scrollok(chatw, TRUE);
            intrflush(inputw, FALSE);
            keypad(inputw, TRUE);

            resize_event = false;
        }
        if (userw_hidden && userw != nullptr)
        {
            delwin(chatw);
            delwin(userw);
            delwin(inputw);

            chatw = newwin(LINES - 1, 0, 0, 0);
            userw = nullptr;
            inputw = newwin(1, 0, LINES - 1, 0);

            scrollok(chatw, TRUE);
            intrflush(inputw, FALSE);
            keypad(inputw, TRUE);
        }
        if (!userw_hidden && userw == nullptr)
        {
            delwin(chatw);
            delwin(inputw);

            int userw_cols = userw_hidden? 0 : std::min(20, COLS / 4);

            chatw = newwin(LINES - 1, COLS - userw_cols, 0, 0);
            userw = newwin(0, userw_cols, 0, COLS - userw_cols - 1);
            inputw = newwin(1, COLS - userw_cols, LINES - 1, 0);

            scrollok(chatw, TRUE);
            scrollok(userw, TRUE);
            intrflush(inputw, FALSE);
            keypad(inputw, TRUE);
        }

        /* update the chat window */
        werase(chatw);
        wmove(chatw, getmaxy(chatw) - 1, 0);
        for (
            size_t i = chat_log_idx;
            (   i < chat_log.size()
             && (i - chat_log_idx) < (size_t)getmaxy(chatw));
            ++i)
        {
            std::string msg = chat_log.at(i);

            /* print the received messages */
            bool bold = false,
                 italic = false,
                 reverse = false,
                 underline = false;
            char fg[3] = {0,0,0},
                 bg[3] = {0,0,0};

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
            wmove(chatw, getcury(chatw) - 1, 0);
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
            for (size_t i = 0; it != it_end && i < user_list_idx; ++i)
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

        /* update the screen */
        wnoutrefresh(chatw);
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
                        if (   event.bstate & BUTTON3_RELEASED
                            || event.bstate & BUTTON3_CLICKED)
                        {
                            userw_hidden = !userw_hidden;
                        }
                        if (event.bstate & BUTTON4_PRESSED)
                        {
                            if (wenclose(chatw, event.y, event.x))
                            {
                                if (  chat_log_idx + 1
                                    < chat_log.size())
                                {
                                    chat_log_idx += 1;
                                }
                            }
                            if (wenclose(userw, event.y, event.x))
                            {
                                if (user_list_idx != 0)
                                {
                                    user_list_idx -= 1;
                                }
                            }
                        }
                        if (event.bstate & BUTTON5_PRESSED)
                        {
                            if (wenclose(chatw, event.y, event.x))
                            {
                                if (chat_log_idx != 0)
                                {
                                    chat_log_idx -= 1;
                                }
                            }
                            if (wenclose(userw, event.y, event.x))
                            {
                                if (  user_list_idx + 1
                                    < channels[
                                        current_channel
                                        ].users.size())
                                {
                                    user_list_idx += 1;
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
    endwin();
    debug("finished");
}

