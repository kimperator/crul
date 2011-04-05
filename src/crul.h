/*  
    Copyright (C) 2011 crul authors,
    
    This file is part of crul.
    
    Butterfly is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Butterfly is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __CRUL_H_
#define __CRUL_H_
#include <curl/curl.h>

#include "../Butterfly/object.h"

/**
 * Crul gives the User an Easy To Use Interface for http post / get requests with cURL and simclist params
 */
typedef struct crul_browser {
	char* url;
	char* proxy;
	char* proxytype;
	char* user_agent;
	char ignore_ssl_cert;
	CURLSH* share;
}crul_browser;

struct MemoryStruct {
	char *memory;
	size_t size;
};

typedef struct crul_response {
	int success;
	int code;
	struct MemoryStruct data;
}crul_response;

//internaly used functions

int crul_encode(char* dst, char* src, unsigned int length);
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

//global functions
void crul_prepare();
void crul_postclean();


//per crul_browser functions
crul_browser* crul_browser_create();
void crul_browser_free(crul_browser* b);

crul_response* crul_browser_open_get(crul_browser* b, char* url);
crul_response* crul_browser_open_post_str(crul_browser* b, char* url, char* post);
crul_response* crul_browser_open_post_str_raw(crul_browser* b, char* url, char* post, long size);
crul_response* crul_browser_open_post(crul_browser* b, char* url, object* post);
void crul_response_free(crul_response* r);

//json-rpc over http(s) methods
object* crul_browser_json_call(crul_browser* b, char* url, char* method, object* params);
object* crul_browser_json_call_get(crul_browser* b, char* url, char* method, object* params);
#endif
