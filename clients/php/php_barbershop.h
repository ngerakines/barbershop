/*
Copyright (c) 2010 Nick Gerakines <nick at gerakines dot net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef PHP_BARBERSHOP_H
#define PHP_BARBERSHOP_H

PHP_METHOD(Barbershop, __construct);
PHP_METHOD(Barbershop, connect);
PHP_METHOD(Barbershop, close);
PHP_METHOD(Barbershop, info);
PHP_METHOD(Barbershop, update);
PHP_METHOD(Barbershop, score);
PHP_METHOD(Barbershop, next);
PHP_METHOD(Barbershop, peek);

#ifdef PHP_WIN32
#define PHP_BARBERSHOP_API __declspec(dllexport)
#else
#define PHP_BARBERSHOP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(barbershop);
PHP_MSHUTDOWN_FUNCTION(barbershop);
PHP_RINIT_FUNCTION(barbershop);
PHP_RSHUTDOWN_FUNCTION(barbershop);
PHP_MINFO_FUNCTION(barbershop);

/* {{{ struct BarbershopSock */
typedef struct BarbershopSock_ {
    php_stream     *stream;
    char           *host;
    unsigned short port;
    long           timeout;
    int            failed;
    int            status;
} BarbershopSock;
/* }}} */

#define barbershop_sock_name "Barbershop Socket Buffer"

#define BARBERSHOP_SOCK_STATUS_FAILED 0
#define BARBERSHOP_SOCK_STATUS_DISCONNECTED 1
#define BARBERSHOP_SOCK_STATUS_UNKNOWN 2
#define BARBERSHOP_SOCK_STATUS_CONNECTED 3

/* properties */
#define BARBERSHOP_NOT_FOUND 0
#define BARBERSHOP_STRING 1
#define BARBERSHOP_SET 2
#define BARBERSHOP_LIST 3


/* {{{ internal function protos */
void add_constant_long(zend_class_entry *ce, char *name, int value);

PHPAPI void barbershop_check_eof(BarbershopSock *barbershop_sock TSRMLS_DC);
PHPAPI BarbershopSock* barbershop_sock_create(char *host, int host_len, unsigned short port, long timeout);
PHPAPI int barbershop_sock_connect(BarbershopSock *barbershop_sock TSRMLS_DC);
PHPAPI int barbershop_sock_disconnect(BarbershopSock *barbershop_sock TSRMLS_DC);
PHPAPI int barbershop_sock_server_open(BarbershopSock *barbershop_sock, int TSRMLS_DC);
PHPAPI char * barbershop_sock_read(BarbershopSock *barbershop_sock, int *buf_len TSRMLS_DC);
PHPAPI int barbershop_sock_write(BarbershopSock *barbershop_sock, char *cmd, size_t sz);
PHPAPI void barbershop_free_socket(BarbershopSock *barbershop_sock);

PHPAPI void barbershop_atomic_increment(INTERNAL_FUNCTION_PARAMETERS, char *keyword TSRMLS_DC);

ZEND_BEGIN_MODULE_GLOBALS(barbershop)
ZEND_END_MODULE_GLOBALS(barbershop)

#define PHP_BARBERSHOP_VERSION "0.2.1"

#endif
