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
#ifndef FOSSIL_NETWORK_SOCKET_H
#define FOSSIL_NETWORK_SOCKET_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*=============================================================================
STRING IDS
=============================================================================*/

/*
Socket Types:
    fossil.net.socket.type.tcp
    fossil.net.socket.type.udp
    fossil.net.socket.type.raw

Address Families:
    fossil.net.family.ipv4
    fossil.net.family.ipv6

Socket Options:
    fossil.net.socket.blocking
    fossil.net.socket.reuseaddr
*/

/*=============================================================================
CORE STRUCTURES
=============================================================================*/

typedef struct fossil_net_socket
{
    void *handle;    /* OS-specific socket */
    char id[64];     /* user-defined ID */
    char type[32];   /* socket type string ID */
    char family[32]; /* address family string ID */
    bool blocking;
} fossil_net_socket_t;

typedef struct fossil_net_address
{
    char ip[64]; /* IPv4 or IPv6 string */
    uint16_t port;
    char family[32];
} fossil_net_address_t;

typedef struct fossil_net_mac
{
    uint8_t bytes[6];
    char string[18]; /* AA:BB:CC:DD:EE:FF */
} fossil_net_mac_t;

/*=============================================================================
THREADING
=============================================================================*/

typedef struct fossil_net_socket_thread
{
    void *handle;
    char id[64];
} fossil_net_socket_thread_t;

typedef void (*fossil_net_socket_thread_fn)(void *arg);

/*=============================================================================
LIFECYCLE
=============================================================================*/

/**
 * @brief Initialize the networking subsystem.
 *
 * This function must be called before any other socket operations.
 * On Windows, it initializes Winsock; on POSIX, it is a no-op.
 *
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_init(void);

/**
 * @brief Shutdown the networking subsystem.
 *
 * Cleans up resources allocated by the networking subsystem.
 * On Windows, it calls WSACleanup; on POSIX, it is a no-op.
 *
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_shutdown(void);

/*=============================================================================
SOCKET MANAGEMENT
=============================================================================*/

/**
 * @brief Create a new socket.
 *
 * Initializes a socket structure with the specified type and address family.
 * Supported types: "fossil.net.socket.type.tcp", "fossil.net.socket.type.udp", "fossil.net.socket.type.raw".
 * Supported families: "fossil.net.family.ipv4", "fossil.net.family.ipv6".
 *
 * @param sock   Pointer to socket structure to initialize.
 * @param type   Socket type string ID.
 * @param family Address family string ID.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_create(
    fossil_net_socket_t *sock,
    const char *type,
    const char *family);

/**
 * @brief Close a socket and release its resources.
 *
 * Closes the underlying OS socket handle and resets the structure.
 *
 * @param sock Pointer to socket structure.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_close(
    fossil_net_socket_t *sock);

/**
 * @brief Set socket blocking or non-blocking mode.
 *
 * Configures the socket to operate in blocking or non-blocking mode.
 *
 * @param sock     Pointer to socket structure.
 * @param blocking true for blocking, false for non-blocking.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_set_blocking(
    fossil_net_socket_t *sock,
    bool blocking);

/*=============================================================================
CONNECTION
=============================================================================*/

/**
 * @brief Bind a socket to a local address and port.
 *
 * Associates the socket with a specific local IP address and port.
 *
 * @param sock Pointer to socket structure.
 * @param addr Pointer to address structure.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_bind(
    fossil_net_socket_t *sock,
    const fossil_net_address_t *addr);

/**
 * @brief Listen for incoming connections on a socket.
 *
 * Marks the socket as a passive socket to accept incoming connection requests.
 *
 * @param sock    Pointer to socket structure.
 * @param backlog Maximum length of the pending connections queue.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_listen(
    fossil_net_socket_t *sock,
    int backlog);

/**
 * @brief Accept an incoming connection on a listening socket.
 *
 * Accepts a new connection and initializes a client socket structure.
 * Optionally fills in the address of the connecting peer.
 *
 * @param server Pointer to listening socket structure.
 * @param client Pointer to client socket structure to initialize.
 * @param addr   Pointer to address structure to receive peer info (optional).
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_accept(
    fossil_net_socket_t *server,
    fossil_net_socket_t *client,
    fossil_net_address_t *addr);

/**
 * @brief Connect a socket to a remote address.
 *
 * Initiates a connection to the specified remote IP address and port.
 *
 * @param sock Pointer to socket structure.
 * @param addr Pointer to remote address structure.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_connect(
    fossil_net_socket_t *sock,
    const fossil_net_address_t *addr);

/*=============================================================================
DATA TRANSFER
=============================================================================*/

/**
 * @brief Send data through a socket.
 *
 * Sends a buffer of data over the socket. The number of bytes actually sent is returned.
 *
 * @param sock  Pointer to socket structure.
 * @param data  Pointer to data buffer to send.
 * @param size  Size of data buffer in bytes.
 * @param sent  Pointer to variable to receive number of bytes sent.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_send(
    fossil_net_socket_t *sock,
    const void *data,
    uint32_t size,
    uint32_t *sent);

/**
 * @brief Receive data from a socket.
 *
 * Receives data from the socket into the provided buffer. The number of bytes received is returned.
 *
 * @param sock     Pointer to socket structure.
 * @param buffer   Pointer to buffer to receive data.
 * @param size     Size of buffer in bytes.
 * @param received Pointer to variable to receive number of bytes received.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_receive(
    fossil_net_socket_t *sock,
    void *buffer,
    uint32_t size,
    uint32_t *received);

/*=============================================================================
ADDRESS UTILITIES
=============================================================================*/

/**
 * @brief Parse an IP address and port into an address structure.
 *
 * Converts a string IP and port number into a fossil_net_address_t structure.
 *
 * @param addr Pointer to address structure to fill.
 * @param ip   String representation of IP address.
 * @param port Port number.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_address_parse(
    fossil_net_address_t *addr,
    const char *ip,
    uint16_t port);

/**
 * @brief Convert an address structure to a string.
 *
 * Formats the address as a string (e.g., "127.0.0.1:8080").
 *
 * @param addr   Pointer to address structure.
 * @param buffer Output buffer for string.
 * @param size   Size of output buffer.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_address_to_string(
    const fossil_net_address_t *addr,
    char *buffer,
    uint32_t size);

/*=============================================================================
DNS / HOST
=============================================================================*/

/**
 * @brief Resolve a hostname to an IP address.
 *
 * Looks up the IP address for the given hostname and fills the address structure.
 *
 * @param hostname Hostname to resolve.
 * @param out_addr Pointer to address structure to fill.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_resolve(
    const char *hostname,
    fossil_net_address_t *out_addr);

/**
 * @brief Get the local machine's hostname.
 *
 * Retrieves the system's hostname into the provided buffer.
 *
 * @param buffer Output buffer for hostname.
 * @param size   Size of output buffer.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_hostname(
    char *buffer,
    uint32_t size);

/*=============================================================================
MAC ADDRESS
=============================================================================*/

/**
 * @brief Get the primary MAC address of the system.
 *
 * Retrieves the hardware MAC address of the primary network interface.
 *
 * @param mac Pointer to MAC address structure to fill.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_mac_get(
    fossil_net_mac_t *mac);

/**
 * @brief Convert a MAC address structure to a string.
 *
 * Formats the MAC address as a string (e.g., "AA:BB:CC:DD:EE:FF").
 *
 * @param mac    Pointer to MAC address structure.
 * @param buffer Output buffer for string.
 * @param size   Size of output buffer.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_mac_to_string(
    const fossil_net_mac_t *mac,
    char *buffer,
    uint32_t size);

/*=============================================================================
POLLING / MULTIPLEXING
=============================================================================*/

/**
 * @brief Poll multiple sockets for readiness.
 *
 * Waits for one or more sockets to become ready for I/O within a timeout period.
 *
 * @param sockets    Array of pointers to socket structures.
 * @param count      Number of sockets in the array.
 * @param timeout_ms Timeout in milliseconds.
 * @return Number of sockets ready, 0 on timeout, or negative on error.
 */
int fossil_net_socket_poll(
    fossil_net_socket_t **sockets,
    uint32_t count,
    uint32_t timeout_ms);

/*=============================================================================
ERROR HANDLING
=============================================================================*/

/**
 * @brief Get the last socket error code.
 *
 * Returns the last error code produced by a socket operation.
 *
 * @return Error code (platform-specific).
 */
int fossil_net_socket_error_last(void);

/**
 * @brief Convert a socket error code to a human-readable string.
 *
 * Returns a string describing the specified error code.
 *
 * @param err Error code.
 * @return Pointer to static string describing the error.
 */
const char *fossil_net_socket_error_string(int err);

/*=============================================================================
UTILITY
=============================================================================*/

/**
 * @brief Sleep for a specified number of milliseconds.
 *
 * Suspends the calling thread for the given duration.
 *
 * @param ms Number of milliseconds to sleep.
 * @return 0 on success, non-zero on failure.
 */
int fossil_net_socket_sleep(uint32_t ms);

#ifdef __cplusplus
}
#include <string>

namespace fossil::net
{

    class Socket
    {
    public:
        /**
         * @brief Default constructor.
         *
         * Initializes the internal socket structure to zero.
         */
        Socket()
        {
            memset(&sock_, 0, sizeof(sock_));
        }

        /**
         * @brief Destructor.
         *
         * Closes the socket and releases any associated resources.
         */
        ~Socket()
        {
            socket_close();
        }

        /**
         * @brief Create a new socket.
         *
         * Initializes the socket with the specified type and address family.
         *
         * @param type   Socket type string ID.
         * @param family Address family string ID.
         * @return 0 on success, non-zero on failure.
         */
        int socket_create(const std::string &type, const std::string &family)
        {
            return fossil_net_socket_create(&sock_, type.c_str(), family.c_str());
        }

        /**
         * @brief Close the socket.
         *
         * Closes the underlying OS socket handle and resets the structure.
         *
         * @return 0 on success, non-zero on failure.
         */
        int socket_close()
        {
            return fossil_net_socket_close(&sock_);
        }

        /**
         * @brief Set blocking or non-blocking mode.
         *
         * Configures the socket to operate in blocking or non-blocking mode.
         *
         * @param blocking true for blocking, false for non-blocking.
         * @return 0 on success, non-zero on failure.
         */
        int socket_set_blocking(bool blocking)
        {
            return fossil_net_socket_set_blocking(&sock_, blocking);
        }

        /**
         * @brief Bind the socket to a local address and port.
         *
         * Associates the socket with a specific local IP address and port.
         *
         * @param addr Pointer to address structure.
         * @return 0 on success, non-zero on failure.
         */
        int socket_bind(const fossil_net_address_t *addr)
        {
            return fossil_net_socket_bind(&sock_, addr);
        }

        /**
         * @brief Listen for incoming connections.
         *
         * Marks the socket as a passive socket to accept incoming connection requests.
         *
         * @param backlog Maximum length of the pending connections queue.
         * @return 0 on success, non-zero on failure.
         */
        int socket_listen(int backlog)
        {
            return fossil_net_socket_listen(&sock_, backlog);
        }

        /**
         * @brief Accept an incoming connection.
         *
         * Accepts a new connection and initializes a client socket structure.
         * Optionally fills in the address of the connecting peer.
         *
         * @param client Reference to client socket to initialize.
         * @param addr   Pointer to address structure to receive peer info (optional).
         * @return 0 on success, non-zero on failure.
         */
        int socket_accept(Socket &client, fossil_net_address_t *addr)
        {
            return fossil_net_socket_accept(&sock_, &client.sock_, addr);
        }

        /**
         * @brief Connect the socket to a remote address.
         *
         * Initiates a connection to the specified remote IP address and port.
         *
         * @param addr Pointer to remote address structure.
         * @return 0 on success, non-zero on failure.
         */
        int socket_connect(const fossil_net_address_t *addr)
        {
            return fossil_net_socket_connect(&sock_, addr);
        }

        /**
         * @brief Send data through the socket.
         *
         * Sends a buffer of data over the socket.
         *
         * @param data Pointer to data buffer to send.
         * @param size Size of data buffer in bytes.
         * @param sent Pointer to variable to receive number of bytes sent.
         * @return 0 on success, non-zero on failure.
         */
        int socket_send(const void *data, uint32_t size, uint32_t *sent)
        {
            return fossil_net_socket_send(&sock_, data, size, sent);
        }

        /**
         * @brief Receive data from the socket.
         *
         * Receives data from the socket into the provided buffer.
         *
         * @param buffer   Pointer to buffer to receive data.
         * @param size     Size of buffer in bytes.
         * @param received Pointer to variable to receive number of bytes received.
         * @return 0 on success, non-zero on failure.
         */
        int socket_receive(void *buffer, uint32_t size, uint32_t *received)
        {
            return fossil_net_socket_receive(&sock_, buffer, size, received);
        }

        /**
         * @brief Get a pointer to the native socket structure.
         *
         * @return Pointer to the internal fossil_net_socket_t structure.
         */
        fossil_net_socket_t *native_handle() { return &sock_; }

        /**
         * @brief Get a const pointer to the native socket structure.
         *
         * @return Const pointer to the internal fossil_net_socket_t structure.
         */
        const fossil_net_socket_t *native_handle() const { return &sock_; }

    private:
        fossil_net_socket_t sock_;
    };

} // namespace fossil

#endif

#endif /* FOSSIL_NETWORK_SOCKET_H */
