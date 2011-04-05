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



#include "src/crul.h"

void testJsonRpcOverHttpGet(crul_browser* b) {
	object* json_params = object_from_json("[1,2]");
	object* resp = crul_browser_json_call_get(b, "http://localhost/jsonrpc/server.php", "sum", json_params);
	object_free(json_params);
	puts(object_to_json(resp, false));
	object_free(resp);
}

void testJsonRpcOverHttpPost(crul_browser* b) {
	object* json_params = object_from_json("[1,2]");
	object* resp = crul_browser_json_call(b, "http://localhost/jsonrpc/server.php", "sum", json_params);
	object_free(json_params);
	puts(object_to_json(resp, false));
	object_free(resp);
}

int main(int argc, char* argv[])
{
	crul_prepare();
	crul_browser* b = crul_browser_create();
	testJsonRpcOverHttpPost(b);
	testJsonRpcOverHttpGet(b);
	crul_browser_free(b);
	crul_postclean();
}
