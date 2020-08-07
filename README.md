
IRC Client
==========
An IRC client with ncurses interface.

The hostname is the 1st argument. The port can be specified by adding a colon
and the port number after the hostname. The 2nd and 3rd arguments are the IRC
username and password, respectively.

Commands are prefixed by a forward slash, any other input is interpreted as a
message to the current channel.

Commands are:
* `HELP` -- Get help
* `JOIN <channel>` -- Join channel
* `PART <channel>` -- Leave channel
* `QUIT [message]` -- Quit IRC (with an optional quit message)
* `QUOTE <command> [args]...` -- Execute a literal IRC command

Right clicking toggles the user list window open and closed.

Examples
--------
Connect to `irc.example.com` on port `1234`.
* `./irc irc.example.com:1234 some-guy P@55W0RD` 
  
Connect to `irc.example.com` on port `6667`.
* `./irc irc.example.com some-guy P@55W0RD`

