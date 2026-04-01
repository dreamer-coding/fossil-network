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
#include "fossil/network/request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*=============================================================================
INTERNAL HELPERS
=============================================================================*/

static int fossil__str_eq(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static void fossil__safe_copy(char *dst, const char *src, size_t size)
{
    if (!dst || size == 0) return;
    if (!src) { dst[0] = '\0'; return; }

    strncpy(dst, src, size - 1);
    dst[size - 1] = '\0';
}

static int fossil__validate_method(const char *method)
{
    if (!method) return -1;
    
    const char *valid_methods[] = {
        "get", "post", "put", "delete", "patch", "head", NULL
    };
    
    for (int i = 0; valid_methods[i]; i++)
    {
        if (fossil__str_eq(method, valid_methods[i]))
            return 0;
    }
    return -1;
}

static int fossil__validate_content_type(const char *type)
{
    if (!type) return -1;
    
    const char *valid_types[] = {
        "text/plain",
        "application/json",
        "application/xml",
        "application/octet-stream",
        NULL
    };
    
    for (int i = 0; valid_types[i]; i++)
    {
        if (fossil__str_eq(type, valid_types[i]))
            return 0;
    }
    return -1;
}

/*=============================================================================
REQUEST BUILDING
=============================================================================*/

int fossil_net_request_init(
    fossil_net_request_t *req,
    const char *method,
    const char *url)
{
    if (!req || !method || !url)
        return -1;

    if (fossil__validate_method(method) != 0)
        return -1;

    memset(req, 0, sizeof(*req));

    fossil__safe_copy(req->method, method, sizeof(req->method));
    fossil__safe_copy(req->url, url, sizeof(req->url));

    fossil__safe_copy(req->content_type, "text/plain", sizeof(req->content_type));
    req->body = NULL;
    req->body_size = 0;
    req->header_count = 0;

    return 0;
}

int fossil_net_request_set_header(
    fossil_net_request_t *req,
    const char *key,
    const char *value)
{
    if (!req || !key || !value)
        return -1;

    /* update if exists */
    for (uint32_t i = 0; i < req->header_count; i++)
    {
        if (fossil__str_eq(req->headers[i].key, key))
        {
            fossil__safe_copy(req->headers[i].value, value, sizeof(req->headers[i].value));
            return 0;
        }
    }

    /* add new */
    if (req->header_count >= 32)
        return -1;

    fossil__safe_copy(req->headers[req->header_count].key, key, sizeof(req->headers[0].key));
    fossil__safe_copy(req->headers[req->header_count].value, value, sizeof(req->headers[0].value));
    req->header_count++;

    return 0;
}

int fossil_net_request_set_body(
    fossil_net_request_t *req,
    const void *data,
    uint32_t size,
    const char *content_type)
{
    if (!req)
        return -1;

    if (content_type && fossil__validate_content_type(content_type) != 0)
        return -1;

    req->body = (void *)data;
    req->body_size = size;

    if (content_type)
        fossil__safe_copy(req->content_type, content_type, sizeof(req->content_type));

    return 0;
}

/*=============================================================================
RESPONSE UTILITIES
=============================================================================*/

int fossil_net_response_get_header(
    const fossil_net_response_t *res,
    const char *key,
    char *buffer,
    uint32_t size)
{
    if (!res || !key || !buffer)
        return -1;

    for (uint32_t i = 0; i < res->header_count; i++)
    {
        if (fossil__str_eq(res->headers[i].key, key))
        {
            fossil__safe_copy(buffer, res->headers[i].value, size);
            return 0;
        }
    }

    return -1;
}

/*=============================================================================
HTTP SERIALIZATION
=============================================================================*/

static int fossil__http_build_request(
    const fossil_net_request_t *req,
    char *buffer,
    uint32_t size,
    uint32_t *out_size)
{
    if (!req || !buffer || !out_size)
        return -1;

    uint32_t offset = 0;

    /* Request line */
    offset += snprintf(buffer + offset, size - offset,
        "%s %s HTTP/1.1\r\n",
        req->method,
        req->url);

    /* Headers */
    for (uint32_t i = 0; i < req->header_count; i++)
    {
        offset += snprintf(buffer + offset, size - offset,
            "%s: %s\r\n",
            req->headers[i].key,
            req->headers[i].value);
    }

    /* Content-Type */
    if (req->body && req->body_size > 0)
    {
        offset += snprintf(buffer + offset, size - offset,
            "Content-Type: %s\r\n",
            req->content_type);

        offset += snprintf(buffer + offset, size - offset,
            "Content-Length: %u\r\n",
            req->body_size);
    }

    /* End headers */
    offset += snprintf(buffer + offset, size - offset, "\r\n");

    /* Body */
    if (req->body && req->body_size > 0)
    {
        if (offset + req->body_size >= size)
            return -1;

        memcpy(buffer + offset, req->body, req->body_size);
        offset += req->body_size;
    }

    *out_size = offset;
    return 0;
}

/*=============================================================================
HTTP RESPONSE PARSER (BASIC)
=============================================================================*/

static int fossil__http_parse_response(
    fossil_net_response_t *res,
    const char *buffer,
    uint32_t size)
{
    if (!res || !buffer)
        return -1;

    memset(res, 0, sizeof(*res));

    const char *ptr = buffer;

    /* Status line */
    sscanf(ptr, "HTTP/%*s %d", &res->status);

    /* Move to headers */
    const char *headers = strstr(ptr, "\r\n");
    if (!headers) return -1;
    headers += 2;

    /* Parse headers */
    while (*headers && !(headers[0] == '\r' && headers[1] == '\n'))
    {
        char key[64] = {0};
        char value[256] = {0};

        sscanf(headers, "%63[^:]: %255[^\r\n]", key, value);

        if (res->header_count < 32)
        {
            fossil__safe_copy(res->headers[res->header_count].key, key, sizeof(key));
            fossil__safe_copy(res->headers[res->header_count].value, value, sizeof(value));
            res->header_count++;
        }

        headers = strstr(headers, "\r\n");
        if (!headers) break;
        headers += 2;
    }

    /* Body */
    const char *body = strstr(headers, "\r\n\r\n");
    if (!body) return 0;
    body += 4;

    uint32_t body_size = size - (uint32_t)(body - buffer);

    res->body = malloc(body_size);
    if (!res->body) return -1;

    memcpy(res->body, body, body_size);
    res->body_size = body_size;

    return 0;
}

/*=============================================================================
EXECUTION
=============================================================================*/

int fossil_net_request_send(
    fossil_net_socket_t *sock,
    const fossil_net_request_t *req,
    fossil_net_response_t *res)
{
    if (!sock || !req || !res)
        return -1;

    char buffer[8192];
    uint32_t req_size = 0;

    if (fossil__http_build_request(req, buffer, sizeof(buffer), &req_size) != 0)
        return -1;

    uint32_t sent = 0;
    if (fossil_net_socket_send(sock, buffer, req_size, &sent) != 0)
        return -1;

    char recv_buf[16384];
    uint32_t received = 0;

    if (fossil_net_socket_receive(sock, recv_buf, sizeof(recv_buf), &received) != 0)
        return -1;

    return fossil__http_parse_response(res, recv_buf, received);
}

/*=============================================================================
LIFECYCLE
=============================================================================*/

void fossil_net_request_free(fossil_net_request_t *req)
{
    if (!req) return;

    /* body is user-owned → do NOT free */
    req->body = NULL;
    req->body_size = 0;
    req->header_count = 0;
}

void fossil_net_response_free(fossil_net_response_t *res)
{
    if (!res) return;

    if (res->body)
    {
        free(res->body);
        res->body = NULL;
    }

    res->body_size = 0;
    res->header_count = 0;
}
