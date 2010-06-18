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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_barbershop.h"
#include <zend_exceptions.h>

static int le_barbershop_sock;
static zend_class_entry *barbershop_ce;
static zend_class_entry *barbershop_exception_ce;
static zend_class_entry *spl_ce_RuntimeException = NULL;

ZEND_DECLARE_MODULE_GLOBALS(barbershop)

static zend_function_entry barbershop_functions[] = {
	 PHP_ME(Barbershop, __construct, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, connect, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, close, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, next, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, peek, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, update, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, score, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Barbershop, info, NULL, ZEND_ACC_PUBLIC)
	 PHP_MALIAS(Barbershop, open, connect, NULL, ZEND_ACC_PUBLIC)
	 {NULL, NULL, NULL}
};

zend_module_entry barbershop_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"barbershop",
	NULL,
	PHP_MINIT(barbershop),
	PHP_MSHUTDOWN(barbershop),
	PHP_RINIT(barbershop),
	PHP_RSHUTDOWN(barbershop),
	PHP_MINFO(barbershop),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_BARBERSHOP_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BARBERSHOP
ZEND_GET_MODULE(barbershop)
#endif

void add_constant_long(zend_class_entry *ce, char *name, int value) {
	zval *constval;
	constval = pemalloc(sizeof(zval), 1);
	INIT_PZVAL(constval);
	ZVAL_LONG(constval, value);
	zend_hash_add(&ce->constants_table, name, 1 + strlen(name), (void*)&constval, sizeof(zval*), NULL);
}

/**
 * This command behave somehow like printf, except that strings need 2 arguments:
 * Their data and their size (strlen).
 * Supported formats are: %d, %i, %s
 */
static int barbershop_cmd_format(char **ret, char *format, ...) {
	char *p, *s;
	va_list ap;

	int total = 0, sz, ret_sz;
	int i, ci;
	unsigned int u;
	double dbl;
	char *double_str;
	int double_len;

	int stage;
	for (stage = 0; stage < 2; ++stage) {
		va_start(ap, format);
		total = 0;
		for (p = format; *p; ) {
			if (*p == '%') {
				switch (*(p+1)) {
					case 's':
						s = va_arg(ap, char*);
						sz = va_arg(ap, int);
						if (stage == 1) {
							memcpy((*ret) + total, s, sz);
						}
						total += sz;
						break;
					case 'f':
						/* use spprintf here */
						dbl = va_arg(ap, double);
						double_len = spprintf(&double_str, 0, "%f", dbl);
						if (stage == 1) {
							memcpy((*ret) + total, double_str, double_len);
						}
						total += double_len;
						efree(double_str);
						break;
					case 'i':
					case 'd':
						i = va_arg(ap, int);
						/* compute display size of integer value */
						sz = 0;
						ci = abs(i);
						while (ci>0) {
								ci = (ci/10);
								sz += 1;
						}
						if (i == 0) { /* log 0 doesn't make sense. */
								sz = 1;
						} else if(i < 0) { /* allow for neg sign as well. */
								sz++;
						}
						if (stage == 1) {
							sprintf((*ret) + total, "%d", i);
						} 
						total += sz;
						break;
				}
				p++;
			} else {
				if (stage == 1) {
					(*ret)[total] = *p;
				}
				total++;
			}
			p++;
		}
		if (stage == 0) {
			ret_sz = total;
			(*ret) = emalloc(ret_sz+1);
		} else {
			(*ret)[ret_sz] = 0;
			return ret_sz;
		}
	}
}

PHPAPI BarbershopSock* barbershop_sock_create(char *host, int host_len, unsigned short port, long timeout) {
	BarbershopSock *barbershop_sock;
	barbershop_sock = emalloc(sizeof *barbershop_sock);
	barbershop_sock->host = emalloc(host_len + 1);
	barbershop_sock->stream = NULL;
	barbershop_sock->status = BARBERSHOP_SOCK_STATUS_DISCONNECTED;

	memcpy(barbershop_sock->host, host, host_len);
	barbershop_sock->host[host_len] = '\0';
	barbershop_sock->port = port;
	barbershop_sock->timeout = timeout;

	return barbershop_sock;
}

PHPAPI int barbershop_sock_connect(BarbershopSock *barbershop_sock TSRMLS_DC) {
	struct timeval tv, *tv_ptr = NULL;
	char *host = NULL, *hash_key = NULL, *errstr = NULL;
	int host_len, err = 0;

	if (barbershop_sock->stream != NULL) {
		barbershop_sock_disconnect(barbershop_sock TSRMLS_CC);
	}

	tv.tv_sec  = barbershop_sock->timeout;
	tv.tv_usec = 0;

	host_len = spprintf(&host, 0, "%s:%d", barbershop_sock->host, barbershop_sock->port);

	if (tv.tv_sec != 0) {
		tv_ptr = &tv;
	}
	barbershop_sock->stream = php_stream_xport_create(host, host_len, ENFORCE_SAFE_MODE, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, hash_key, tv_ptr, NULL, &errstr, &err);

	efree(host);

	if (!barbershop_sock->stream) {
		efree(errstr);
		return -1;
	}

	php_stream_auto_cleanup(barbershop_sock->stream);

	if (tv.tv_sec != 0) {
		php_stream_set_option(barbershop_sock->stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
	}
	php_stream_set_option(barbershop_sock->stream, PHP_STREAM_OPTION_WRITE_BUFFER, PHP_STREAM_BUFFER_NONE, NULL);

	barbershop_sock->status = BARBERSHOP_SOCK_STATUS_CONNECTED;
	return 0;
}

PHPAPI int barbershop_sock_server_open(BarbershopSock *barbershop_sock, int force_connect TSRMLS_DC) {
	int res = -1;
	switch (barbershop_sock->status) {
		case BARBERSHOP_SOCK_STATUS_DISCONNECTED:
			return barbershop_sock_connect(barbershop_sock TSRMLS_CC);
		case BARBERSHOP_SOCK_STATUS_CONNECTED:
			res = 0;
		break;
		case BARBERSHOP_SOCK_STATUS_UNKNOWN:
			if (force_connect > 0 && barbershop_sock_connect(barbershop_sock TSRMLS_CC) < 0) {
				res = -1;
			} else {
				res = 0;
				barbershop_sock->status = BARBERSHOP_SOCK_STATUS_CONNECTED;
			}
		break;
	}
	return res;
}

PHPAPI int barbershop_sock_disconnect(BarbershopSock *barbershop_sock TSRMLS_DC) {
	int res = 0;
	if (barbershop_sock->stream != NULL) {
		//barbershop_sock_write(barbershop_sock, "QUIT", sizeof("QUIT") - 1);
		barbershop_sock->status = BARBERSHOP_SOCK_STATUS_DISCONNECTED;
		php_stream_close(barbershop_sock->stream);
		barbershop_sock->stream = NULL;
		res = 1;
	}
	return res;
}

PHPAPI char *barbershop_sock_read(BarbershopSock *barbershop_sock, int *buf_len TSRMLS_DC) {
	char inbuf[1024];
	char *resp = NULL;

	barbershop_check_eof(barbershop_sock TSRMLS_CC);
	php_stream_gets(barbershop_sock->stream, inbuf, 1024);

	switch (inbuf[0]) {
		case '-':
			return NULL;
		case '+':
		case ':':
			*buf_len = strlen(inbuf) - 2;
			if (*buf_len >= 2) {
				resp = emalloc(1+*buf_len);
				memcpy(resp, inbuf, *buf_len);
				resp[*buf_len] = 0;
				return resp;
			} else {
				printf("protocol error \n");
				return NULL;
			}
		default:
			printf("protocol error, got '%c' as reply type byte\n", inbuf[0]);
	}
	return NULL;
}

PHPAPI int barbershop_sock_write(BarbershopSock *barbershop_sock, char *cmd, size_t sz) {
	barbershop_check_eof(barbershop_sock TSRMLS_CC);
	return php_stream_write(barbershop_sock->stream, cmd, sz);
	return 0;
}

PHPAPI void barbershop_check_eof(BarbershopSock *barbershop_sock TSRMLS_DC) {
	int eof = php_stream_eof(barbershop_sock->stream);
	while (eof) {
		barbershop_sock->stream = NULL;
		barbershop_sock_connect(barbershop_sock TSRMLS_CC);
		eof = php_stream_eof(barbershop_sock->stream);
	}
}

PHPAPI int barbershop_sock_get(zval *id, BarbershopSock **barbershop_sock TSRMLS_DC) {
	zval **socket;
	int resource_type;

	if (Z_TYPE_P(id) != IS_OBJECT || zend_hash_find(Z_OBJPROP_P(id), "socket", sizeof("socket"), (void **) &socket) == FAILURE) {
		return -1;
	}

	*barbershop_sock = (BarbershopSock *) zend_list_find(Z_LVAL_PP(socket), &resource_type);
	if (!*barbershop_sock || resource_type != le_barbershop_sock) {
			return -1;
	}

	return Z_LVAL_PP(socket);
}

/**
 * barbershop_free_socket
 */
PHPAPI void barbershop_free_socket(BarbershopSock *barbershop_sock) {
	efree(barbershop_sock->host);
	efree(barbershop_sock);
}

PHPAPI zend_class_entry *barbershop_get_exception_base(int root TSRMLS_DC) {
#if HAVE_SPL
	if (!root) {
			if (!spl_ce_RuntimeException) {
					zend_class_entry **pce;
					if (zend_hash_find(CG(class_table), "runtimeexception", sizeof("RuntimeException"), (void **) &pce) == SUCCESS) {
							spl_ce_RuntimeException = *pce;
							return *pce;
					}
			} else {
					return spl_ce_RuntimeException;
			}
	}
#endif
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
	return zend_exception_get_default();
#else
	return zend_exception_get_default(TSRMLS_C);
#endif
}

static void barbershop_destructor_barbershop_sock(zend_rsrc_list_entry * rsrc TSRMLS_DC) {
	BarbershopSock *barbershop_sock = (BarbershopSock *) rsrc->ptr;
	barbershop_sock_disconnect(barbershop_sock TSRMLS_CC);
	barbershop_free_socket(barbershop_sock);
}

PHP_MINIT_FUNCTION(barbershop) {
	zend_class_entry barbershop_class_entry;
	INIT_CLASS_ENTRY(barbershop_class_entry, "Barbershop", barbershop_functions);
	barbershop_ce = zend_register_internal_class(&barbershop_class_entry TSRMLS_CC);

	zend_class_entry barbershop_exception_class_entry;
	INIT_CLASS_ENTRY(barbershop_exception_class_entry, "BarbershopException", NULL);
	barbershop_exception_ce = zend_register_internal_class_ex(
		&barbershop_exception_class_entry,
		barbershop_get_exception_base(0 TSRMLS_CC),
		NULL TSRMLS_CC
	);

	le_barbershop_sock = zend_register_list_destructors_ex(
		barbershop_destructor_barbershop_sock,
		NULL,
		barbershop_sock_name, module_number
	);
	// XXX: Scrub these
	add_constant_long(barbershop_ce, "BARBERSHOP_NOT_FOUND", BARBERSHOP_NOT_FOUND);
	add_constant_long(barbershop_ce, "BARBERSHOP_STRING", BARBERSHOP_STRING);
	add_constant_long(barbershop_ce, "BARBERSHOP_SET", BARBERSHOP_SET);
	add_constant_long(barbershop_ce, "BARBERSHOP_LIST", BARBERSHOP_LIST);
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(barbershop) {
	return SUCCESS;
}

PHP_RINIT_FUNCTION(barbershop) {
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(barbershop) {
	return SUCCESS;
}

PHP_MINFO_FUNCTION(barbershop) {
	php_info_print_table_start();
	php_info_print_table_header(2, "Barberbshop Support", "enabled");
	php_info_print_table_row(2, "Version", PHP_BARBERSHOP_VERSION);
	php_info_print_table_end();
}

PHP_METHOD(Barbershop, __construct) {
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		RETURN_FALSE;
	}
}

PHP_METHOD(Barbershop, connect) {
	zval *object;
	int host_len, id;
	char *host = NULL;
	long port;

	struct timeval timeout = {0L, 0L};
	BarbershopSock *barbershop_sock	 = NULL;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osl|l", &object, barbershop_ce, &host, &host_len, &port, &timeout.tv_sec) == FAILURE) {
		RETURN_FALSE;
	}

	if (timeout.tv_sec < 0L || timeout.tv_sec > INT_MAX) {
		zend_throw_exception(barbershop_exception_ce, "Invalid timeout", 0 TSRMLS_CC);
		RETURN_FALSE;
	}

	barbershop_sock = barbershop_sock_create(host, host_len, port, timeout.tv_sec);

	if (barbershop_sock_server_open(barbershop_sock, 1 TSRMLS_CC) < 0) {
		barbershop_free_socket(barbershop_sock);
		zend_throw_exception_ex(
			barbershop_exception_ce,
			0 TSRMLS_CC,
			"Can't connect to %s:%d",
			host,
			port
		);
		RETURN_FALSE;
	}

	id = zend_list_insert(barbershop_sock, le_barbershop_sock);
	add_property_resource(object, "socket", id);

	RETURN_TRUE;
}

PHP_METHOD(Barbershop, close) {
	zval *object;
	BarbershopSock *barbershop_sock = NULL;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, barbershop_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (barbershop_sock_get(object, &barbershop_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	if (barbershop_sock_disconnect(barbershop_sock TSRMLS_CC)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHPAPI void barbershop_boolean_response(INTERNAL_FUNCTION_PARAMETERS, BarbershopSock *barbershop_sock TSRMLS_DC) {
	char *response;
	int response_len;
	char ret;

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	ret = response[0];
	efree(response);

	if (ret == '+') {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHPAPI void barbershop_long_response(INTERNAL_FUNCTION_PARAMETERS, BarbershopSock *barbershop_sock TSRMLS_DC) {
	char *response;
	int response_len;

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	if(response[0] == ':') {
		long ret = atol(response + 1);
		efree(response);
		RETURN_LONG(ret);
	} else {
		efree(response);
		RETURN_FALSE;
	}
}

PHPAPI void barbershop_bulk_double_response(INTERNAL_FUNCTION_PARAMETERS, BarbershopSock *barbershop_sock TSRMLS_DC) {

	char *response;
	int response_len;	 

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	double ret = atof(response);
	efree(response);
	RETURN_DOUBLE(ret);
}
	
PHPAPI void barbershop_1_response(INTERNAL_FUNCTION_PARAMETERS, BarbershopSock *barbershop_sock TSRMLS_DC) {

	char *response;
	int response_len;
	char ret;

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	ret = response[1];
	efree(response);

	if (ret == '1') {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_METHOD(Barbershop, update) {
	zval *object;
	BarbershopSock *barbershop_sock;
	char *key = NULL, *val = NULL, *cmd, *response;
	int key_len, val_len, cmd_len, response_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss", &object, barbershop_ce, &key, &key_len, &val, &val_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (barbershop_sock_get(object, &barbershop_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	cmd_len = barbershop_cmd_format(&cmd, "UPDATE %s %s\r\n", key, key_len, val, val_len);
	if (barbershop_sock_write(barbershop_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		RETURN_FALSE;
	}
	efree(cmd);

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	RETURN_STRINGL(response, response_len, 0);
}

PHP_METHOD(Barbershop, score) {
	zval *object;
	BarbershopSock *barbershop_sock;
	char *key = NULL, *cmd, *response;
	int key_len, cmd_len, response_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &object, barbershop_ce, &key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (barbershop_sock_get(object, &barbershop_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	cmd_len = barbershop_cmd_format(&cmd, "SCORE %s\r\n", key, key_len);
	if (barbershop_sock_write(barbershop_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		RETURN_FALSE;
	}
	efree(cmd);

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	RETURN_STRINGL(response, response_len, 0);
}

PHP_METHOD(Barbershop, next) {
	zval *object;
	BarbershopSock *barbershop_sock;
	char *key = NULL, *cmd, *response;
	int key_len, cmd_len, response_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, barbershop_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (barbershop_sock_get(object, &barbershop_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	cmd_len = spprintf(&cmd, 0, "NEXT\r\n");
	if (barbershop_sock_write(barbershop_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		RETURN_FALSE;
	}
	efree(cmd);

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	RETURN_STRINGL(response, response_len, 0);
}

PHP_METHOD(Barbershop, peek) {
	zval *object;
	BarbershopSock *barbershop_sock;
	char *key = NULL, *cmd, *response;
	int key_len, cmd_len, response_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, barbershop_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (barbershop_sock_get(object, &barbershop_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	cmd_len = spprintf(&cmd, 0, "PEEK\r\n");
	if (barbershop_sock_write(barbershop_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		RETURN_FALSE;
	}
	efree(cmd);

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	RETURN_STRINGL(response, response_len, 0);
}

PHP_METHOD(Barbershop, info) {

	zval *object;
	BarbershopSock *barbershop_sock;

	char cmd[] = "INFO\r\n", *response, *key;
	int cmd_len = sizeof(cmd)-1, response_len;
	long ttl;
	char *cur, *pos, *value;
	int is_numeric;
	char *p;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, barbershop_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (barbershop_sock_get(object, &barbershop_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	if (barbershop_sock_write(barbershop_sock, cmd, cmd_len) < 0) {
		RETURN_FALSE;
	}

	if ((response = barbershop_sock_read(barbershop_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);

	cur = response;
	while(1) {
		/* key */
		pos = strchr(cur, ':');
		if(pos == NULL) {
			break;
		}
		key = emalloc(pos - cur + 1);
		memcpy(key, cur, pos-cur);
		key[pos-cur] = 0;

		/* value */
		cur = pos + 1;
		pos = strchr(cur, '\r');
		if(pos == NULL) {
			break;
		}
		value = emalloc(pos - cur + 1);
		memcpy(value, cur, pos-cur);
		value[pos-cur] = 0;
		pos += 2; /* \r, \n */
		cur = pos;

		is_numeric = 1;
		for(p = value; *p; ++p) {
			if(*p < '0' || *p > '9') {
				is_numeric = 0;
				break;
			}
		}

		if(is_numeric == 1) {
			add_assoc_long(return_value, key, atol(value));
			efree(value);
		} else {
			add_assoc_string(return_value, key, value, 0);
		}
		efree(key);
	}
}

/* vim: set tabstop=4 expandtab: */