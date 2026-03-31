/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#ifndef FOSSIL_NETWORK_SERVER_H
#define FOSSIL_NETWORK_SERVER_H

#include "socket.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*=============================================================================
SERVER INTERFACE
=============================================================================*/

/**
 * @brief Opaque server handle.
 */
typedef struct fossil_net_server fossil_net_server_t;

/**
 * @brief Create a new server instance.
 *
 * Initializes a server to listen on the specified address and port.
 *
 * @param type   Socket type string ID ("tcp", "udp").
 * @param family Address family string ID ("ipv4", "ipv6").
 * @param addr   Local address to bind to (can be NULL for any).
 * @param port   Local port to bind to.
 * @return Pointer to server instance, or NULL on failure.
 */
fossil_net_server_t *fossil_net_server_create(
    const char *type,
    const char *family,
    const char *addr,
    uint16_t port);

/**
 * @brief Destroy a server instance and release its resources.
 *
 * @param server Pointer to server instance.
 */
void fossil_net_server_destroy(fossil_net_server_t *server);

/**
 * @brief Start listening for incoming connections.
 *
 * @param server  Pointer to server instance.
 * @param backlog Maximum length of the pending connections queue.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_server_listen(
    fossil_net_server_t *server,
    int backlog);

/**
 * @brief Accept an incoming client connection.
 *
 * Blocks or polls for a new client connection and returns a socket for communication.
 *
 * @param server Pointer to server instance.
 * @param client_sock Pointer to socket structure to initialize.
 * @param client_addr Pointer to address structure to receive client info (optional).
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_server_accept(
    fossil_net_server_t *server,
    fossil_net_socket_t *client_sock,
    fossil_net_address_t *client_addr);

/**
 * @brief Get the local address the server is bound to.
 *
 * @param server Pointer to server instance.
 * @param addr   Pointer to address structure to fill.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_server_get_address(
    fossil_net_server_t *server,
    fossil_net_address_t *addr);

/**
 * @brief Set blocking or non-blocking mode for the server socket.
 *
 * @param server   Pointer to server instance.
 * @param blocking true for blocking, false for non-blocking.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_server_set_blocking(
    fossil_net_server_t *server,
    bool blocking);

#ifdef __cplusplus
}
#include <string>
#include <cstring>

namespace fossil::net
{



} // namespace fossil

#endif

#endif /* FOSSIL_NETWORK_SOCKET_H */
