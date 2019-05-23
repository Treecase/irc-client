
IRC CLIENT
==========

  A barebones IRC client. Responds automatically to PING and VERSION messages,
and allows the user to send IRC commands by typing them in the terminal. Uses the
username "user\_name" and password "very\_secure\_password" by default, but these
can be changed in main.c. It requires the server name to be argument 1, and you
can specify a port number as the optional 2nd argument. For example,
`./irc irc.example.com 6697` will connect to irc.example.com on port 6697.

