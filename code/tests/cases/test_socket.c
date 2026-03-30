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
#include "fossil/network/framework.h"
#include <fossil/pizza/framework.h>

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_SUITE(c_socket_fixture);

FOSSIL_SETUP(c_socket_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_socket_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(c_socket_test_socket_create_types_and_families) {
    fossil_net_socket_t sock;
    // TCP IPv4
    int rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.tcp", "fossil.net.family.ipv4");
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_close(&sock);
    // TCP IPv6
    rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.tcp", "fossil.net.family.ipv6");
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_close(&sock);
    // UDP IPv4
    rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.udp", "fossil.net.family.ipv4");
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_close(&sock);
    // UDP IPv6
    rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.udp", "fossil.net.family.ipv6");
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_close(&sock);
    // RAW IPv4
    rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.raw", "fossil.net.family.ipv4");
    // RAW sockets may require privileges, so allow failure
    ASSUME_ITS_TRUE(rc == 0 || rc == -1);
    if (rc == 0) fossil_net_socket_close(&sock);
}

FOSSIL_TEST(c_socket_test_socket_blocking_option) {
    fossil_net_socket_t sock;
    int rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.tcp", "fossil.net.family.ipv4");
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_set_blocking(&sock, false);
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_set_blocking(&sock, true);
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_close(&sock);
}

FOSSIL_TEST(c_socket_test_socket_address_parse_and_to_string) {
    fossil_net_address_t addr;
    int rc = fossil_net_socket_address_parse(&addr, "127.0.0.1", 8080);
    ASSUME_ITS_TRUE(rc == 0);
    char buf[128];
    rc = fossil_net_socket_address_to_string(&addr, buf, sizeof(buf));
    ASSUME_ITS_TRUE(rc == 0);
    ASSUME_ITS_TRUE(strstr(buf, "127.0.0.1") != NULL);
}

FOSSIL_TEST(c_socket_test_socket_bind_and_listen_ipv6) {
    fossil_net_socket_t sock;
    fossil_net_address_t addr;
    int rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.tcp", "fossil.net.family.ipv6");
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_address_parse(&addr, "::1", 0);
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_bind(&sock, &addr);
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_listen(&sock, 1);
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_close(&sock);
}

FOSSIL_TEST(c_socket_test_socket_send_receive_loopback) {
    fossil_net_socket_t server, client, peer;
    fossil_net_address_t addr, peer_addr;
    int rc = fossil_net_socket_create(&server, "fossil.net.socket.type.tcp", "fossil.net.family.ipv4");
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_address_parse(&addr, "127.0.0.1", 0);
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_bind(&server, &addr);
    ASSUME_ITS_TRUE(rc == 0);
    rc = fossil_net_socket_listen(&server, 1);
    ASSUME_ITS_TRUE(rc == 0);

    // Get the port assigned
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname((int)(intptr_t)server.handle, (struct sockaddr *)&sin, &len);
    uint16_t port = ntohs(sin.sin_port);

    // Connect client
    rc = fossil_net_socket_create(&client, "fossil.net.socket.type.tcp", "fossil.net.family.ipv4");
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_address_t connect_addr;
    fossil_net_socket_address_parse(&connect_addr, "127.0.0.1", port);
    rc = fossil_net_socket_connect(&client, &connect_addr);
    ASSUME_ITS_TRUE(rc == 0);

    // Accept on server
    rc = fossil_net_socket_accept(&server, &peer, &peer_addr);
    ASSUME_ITS_TRUE(rc == 0);

    // Send/receive
    const char *msg = "hello";
    uint32_t sent = 0, recvd = 0;
    rc = fossil_net_socket_send(&client, msg, 5, &sent);
    ASSUME_ITS_TRUE(rc == 0 && sent == 5);
    char buf[16] = {0};
    rc = fossil_net_socket_receive(&peer, buf, sizeof(buf), &recvd);
    ASSUME_ITS_TRUE(rc == 0 && recvd == 5);
    ASSUME_ITS_TRUE(strncmp(buf, "hello", 5) == 0);

    fossil_net_socket_close(&peer);
    fossil_net_socket_close(&client);
    fossil_net_socket_close(&server);
}

FOSSIL_TEST(c_socket_test_socket_mac_get_and_to_string) {
    fossil_net_mac_t mac;
    int rc = fossil_net_socket_mac_get(&mac);
    ASSUME_ITS_TRUE(rc == 0);
    char buf[32];
    rc = fossil_net_socket_mac_to_string(&mac, buf, sizeof(buf));
    ASSUME_ITS_TRUE(rc == 0);
    ASSUME_ITS_TRUE(strlen(buf) >= 11); // "AA:BB:CC:DD:EE:FF"
}

FOSSIL_TEST(c_socket_test_socket_resolve_and_hostname) {
    fossil_net_address_t addr;
    int rc = fossil_net_socket_resolve("localhost", &addr);
    ASSUME_ITS_TRUE(rc == 0);
    char hostname[128];
    rc = fossil_net_socket_hostname(hostname, sizeof(hostname));
    ASSUME_ITS_TRUE(rc == 0);
    ASSUME_ITS_TRUE(strlen(hostname) > 0);
}

FOSSIL_TEST(c_socket_test_socket_poll_timeout) {
    fossil_net_socket_t sock;
    int rc = fossil_net_socket_create(&sock, "fossil.net.socket.type.tcp", "fossil.net.family.ipv4");
    ASSUME_ITS_TRUE(rc == 0);
    fossil_net_socket_t *socks[1] = { &sock };
    int ready = fossil_net_socket_poll(socks, 1, 100);
    ASSUME_ITS_TRUE(ready == 0 || ready == -1);
    fossil_net_socket_close(&sock);
}

FOSSIL_TEST(c_socket_test_socket_error_string) {
    int err = fossil_net_socket_error_last();
    const char *msg = fossil_net_socket_error_string(err);
    ASSUME_ITS_TRUE(msg != NULL);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_socket_tests) {
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_create_types_and_families);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_blocking_option);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_address_parse_and_to_string);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_bind_and_listen_ipv6);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_send_receive_loopback);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_mac_get_and_to_string);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_resolve_and_hostname);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_poll_timeout);
    FOSSIL_TEST_ADD(c_socket_fixture, c_socket_test_socket_error_string);

    FOSSIL_TEST_REGISTER(c_socket_fixture);
} // end of tests
