/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef UTIL_SOCKETS_HPP
#define UTIL_SOCKETS_HPP

#include <string>


/** Open a connection to HOSTNAME:PORT_NUMBER. */
int get_tcp_socket(std::string const &hostname, std::string const &port_number);

/** Read all available data from SOCKET. */
std::string read_socket(int socket);

/** Write DATA to SOCKET. */
void write_socket(int socket, std::string const &data);


#endif
