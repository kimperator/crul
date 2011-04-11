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

#include "crul.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
/* not reserved characters due to rfc 3986 */
static char crul_urlencode_unreserved[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMOPQRSTUVWXYZ0123456789_.-~";

static size_t crul_callback_memory(void *ptr, size_t size, size_t nmemb, void *data) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;
	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */ 
		printf("not enough memory (realloc returned NULL)\n");
		exit(EXIT_FAILURE);
	}
	memcpy(&(mem->memory[mem->size]), ptr, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

static size_t crul_callback_file(void* ptr, size_t size, size_t nmemb, FILE* fp) {
	return fwrite(ptr, size, nmemb, fp);
}

/* 
 * curl_easy_encode in my router sdk broken and escapes '_' -.-
 *
 * dst could be up to 3 times as big as length
 * */
int crul_encode(char* dst, char* src, unsigned int length) {
	int wrote = 0;
	register const unsigned char* s=(const unsigned char*) src; /*force this var to be stored in register*/
	if(!dst || !src)
		return -1; /*if dst or src is null, return -1*/

	unsigned int i;
	for(i=0; i < length; i++) {
		if(strchr(crul_urlencode_unreserved, s[i])) {
			dst[wrote++]=s[i]; /*simple copy char*/
		}
		else {
			/* escape it with % and hex value */
			sprintf(dst+wrote, "%%%X", s[i]);
			wrote += 3;
		}
	}
	/*terminate c string*/
	dst[wrote++] = 0;
	return wrote;
}

void crul_prepare() {
	curl_global_init(CURL_GLOBAL_ALL);
	srand(time(NULL));
}

void crul_postclean() {
	curl_global_cleanup();
}

crul_browser* crul_browser_create() {
	crul_browser* b = (crul_browser*) malloc(sizeof(crul_browser));
	memset(b, 0x0, sizeof(crul_browser));
	b->user_agent = strdup("Mozilla/4.0 (compatible; MSIE 7.0b; Windows NT 6.0)");
	b->ignore_ssl_cert = 1;
	b->url = NULL;
	b->share = curl_share_init();
	curl_share_setopt(b->share, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
	return b;
}

void crul_browser_set_proxy(crul_browser* b, char* proxy, char* proxytype) {
	if(b->proxy)
		free(b->proxy);
	if(b->proxytype)
		free(b->proxytype);
	if(proxy)
		if(b->proxy)
			free(b->proxy);
		b->proxy = strdup(proxy);
	if(proxytype)
		if(b->proxytype)
			free(b->proxytype);
		b->proxytype = strdup(proxytype);
}

crul_response* response_create() {
	crul_response* r = (crul_response*) malloc(sizeof(crul_response));
	memset(r, 0x0, sizeof(crul_response));
	return r;
}

void crul_response_free(crul_response* r) {
	if(r) {
		if(r->data.memory)
		{
			free(r->data.memory);
			r->data.memory = NULL;
		}
		free(r);
		r = NULL;
	}
}

void crul_browser_set_url(crul_browser* b, char* url) {
	if(b->url)
		free(b->url);
	b->url = strdup(url);
}

CURL* crul_prepare_request(crul_browser* b, char* url, char handle_redirect, crul_response* response, FILE* fp) {
	CURL *request = curl_easy_init();

	if(url)
		//sets url
		curl_easy_setopt(request, CURLOPT_URL, url);
	
	if(handle_redirect)
		//sets auto handles redirect
		curl_easy_setopt(request, CURLOPT_FOLLOWLOCATION, 1);

	if(fp) {
		/* sets file callback */
		curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, crul_callback_file);
		curl_easy_setopt(request, CURLOPT_WRITEDATA, fp);
	}
	if(response) {
		/*sets mem callback*/
		curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, crul_callback_memory);
		/* we pass our 'chunk' to the callback function */
		curl_easy_setopt(request, CURLOPT_WRITEDATA, &(response->data));
	}
	
	if(b->share)
		//share cookies and dns data between requests
		curl_easy_setopt(request, CURLOPT_SHARE, b->share);

	if(b->user_agent)
		/* some servers don't like requests that are made without a user-agent
	 	*      field, so we provide one */ 
		curl_easy_setopt(request, CURLOPT_USERAGENT, b->user_agent);
	return request;
}

crul_response* crul_browser_open_get(crul_browser* b, char* url) {
	crul_response* ret = response_create();
	crul_browser_set_url(b, url);
	CURL* request = crul_prepare_request(b, url, 1, ret, NULL);
	ret->success = curl_easy_perform(request);
	curl_easy_cleanup(request);
	return ret;
}

crul_response* crul_browser_open_post_str(crul_browser* b, char* url, char* post) {
	long length = 0;
	if(post)
		length = strlen(post);
	return crul_browser_open_post_str_raw(b, url, post, length);
}

crul_response* crul_browser_open_post_str_raw(crul_browser* b, char* url, char* post, long size) {
	crul_response* ret = response_create();
	crul_browser_set_url(b, url);
	CURL* request = crul_prepare_request(b, url, 1, ret, NULL);
	if(post) {
		curl_easy_setopt(request, CURLOPT_POST, 1);
		curl_easy_setopt(request, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(request, CURLOPT_POSTFIELDSIZE_LARGE, size); 
	}
	ret->success = curl_easy_perform(request);
	curl_easy_cleanup(request);
	return ret;
}

char* crul_object_map_to_post(object* post) {
	if(object_type(post) != OBJECT_MAP) {
		return NULL;
	}
	int size = 256;
	char* data = calloc(size, sizeof(char));
	int left = size-1;
	object_iterator* itr = object_iterate(post);
	while(object_iterator_hasnext(itr))
	{
		object* rec = object_iterator_getnext(itr);
		object* keyobj = object_list_get(rec, 0);
		char* key = object_str_get(keyobj);
		object_free(keyobj);
		object* valueobj = object_list_get(rec, 1);
		char* key_encoded = malloc(strlen(key)*3+1);
		int key_size = crul_encode(key_encoded, key, strlen(key));
		curl_free(key);
		char* value = object_str_get(valueobj);
		char* value_encoded = malloc(strlen(value)*3+1);
		int value_size = crul_encode(value_encoded, value, strlen(value));
		object_free(rec);
		curl_free(value);
		object_free(rec);
		if(left < key_size + value_size + 2) {
			data = realloc(data, size + key_size + value_size + 2);
		}
			
		strcat(data, key_encoded);
		strcat(data, "=");
		strcat(data, value_encoded);
		strcat(data, "&");
		left = size-strlen(data) + 2;


		curl_free(key_encoded);
		curl_free(value_encoded);

    }
    object_iterator_free(itr);
    if(data && strlen(data) > 0)
	data[strlen(data)-1] = 0; //remove last '&'
    return data;
}

crul_response* crul_browser_open_post(crul_browser* b, char* url, object* post) {
    char* data = crul_object_map_to_post(post);
    crul_response* ret = NULL;
    if(data)
	ret = crul_browser_open_post_str(b, url, data);
    curl_free(data);
    return ret;
}

object* crul_browser_json_call(crul_browser* b, char* url, char* method, object* params) {
	char* request_stub = "{\"jsonrpc\": \"2.0\", \"method\": null, \"id\": null}";
	object* req = object_from_json(request_stub);
	
	object* key = object_str("method");
	object* val = object_str(method);
	object_map_set(req, key, val);
	object_free(key);
	object_free(val);

	key = object_str("id");
	val = object_int(rand());
	object_map_set(req, key, val);
	object_free(key);
	object_free(val);
	
	if(!(object_type(params) == OBJECT_NONE)) {
		key = object_str("params");
		object_map_set(req, key, params);
		object_free(key);
	}
	char* req_str = object_to_json(req, false);
	crul_response* c_resp = crul_browser_open_post_str(b, url, req_str);
	object* resp = object_from_json(c_resp->data.memory);
	crul_response_free(c_resp);
	free(req_str);
	object_free(req);
	return resp;
}

object* crul_browser_json_call_get(crul_browser* b, char* url, char* method, object* params) {
	unsigned int url_length = strlen(url) + 24;
	char* params_str = NULL;
	if(object_type(params) != OBJECT_NONE) {
		params_str = object_to_json(params, false);
		url_length += 9 + strlen(params_str); 
	}
	unsigned int id = rand();
	char id_str[16];
	sprintf(id_str, "%d", id);
	url_length += strlen(id_str);
	url_length += strlen(method);
	char* full_url = malloc(url_length);
	sprintf(full_url, "%s?jsonrpc=2.0&id=%s&method=%s", url, id_str, method);
	if(params_str) {
		strcat(full_url, "&params=");
		strcat(full_url, params_str);
	}
	crul_response* ret = crul_browser_open_get(b, full_url);
	object* resp = object_from_json(ret->data.memory);
	crul_response_free(ret);
	curl_free(params_str);
	curl_free(full_url);
	return resp;
}

void crul_browser_free(crul_browser* b) {
	if(b->share) {
		curl_share_cleanup(b->share);
		b->share = NULL;
	}
	if(b->user_agent) {
		free(b->user_agent);
		b->user_agent = NULL;
	}
	if(b->url) {
		free(b->url);
		b->url = NULL;
	}
	if(b->proxy) {
		free(b->proxy);
		b->proxy = NULL;
	}
	if(b->proxytype) {
		free(b->proxytype);
		b->proxytype = NULL;
	}
	free(b);
	b = NULL;
}

char crul_browser_download_get(crul_browser* b, char* url, char* filename) {
	char ret;
	crul_browser_set_url(b, url);
	FILE* fp = fopen(filename, "wb");
	CURL* request = crul_prepare_request(b, url, 1, NULL, fp);
	ret = curl_easy_perform(request);
	fclose(fp);
	curl_easy_cleanup(request);
	return ret == 0;
}

char crul_browser_download_post(crul_browser* b, char* url, object* post, char* filename) {
    char* data = crul_object_map_to_post(post);
    char ret = -1;
    if(data)
	ret = crul_browser_download_post_str(b, url, data, filename);
    curl_free(data);
    return ret;

}

char crul_browser_download_post_str(crul_browser* b, char* url, char* post, char* filename) {
	unsigned int length = strlen(post);
	return crul_browser_download_post_str_raw(b, url, post, length, filename);
}

char crul_browser_download_post_str_raw(crul_browser* b, char* url, char* post, unsigned int length, char* filename) {
	char ret;
	crul_browser_set_url(b, url);
	FILE* fp = fopen(filename, "wb");
	CURL* request = crul_prepare_request(b, url, 1, NULL, fp);
	if(post) {
		curl_easy_setopt(request, CURLOPT_POST, 1);
		curl_easy_setopt(request, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(request, CURLOPT_POSTFIELDSIZE_LARGE, length); 
	}
	ret = curl_easy_perform(request);
	fclose(fp);
	curl_easy_cleanup(request);
	return ret == 0;
}
