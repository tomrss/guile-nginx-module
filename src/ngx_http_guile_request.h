#ifndef _NGX_HTTP_GUILE_REQUEST_INCLUDED_
#define _NGX_HTTP_GUILE_REQUEST_INCLUDED_
/* Guile NGINX Module
 * Copyright (C) 2023-2024 Tommaso Rossi
 *
 * This file is part of Guile NGINX Module.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
// ngx must be included first
#include <libguile.h>

/* Embed nginx http request structure into guile */
typedef struct
{
  ngx_http_request_t *http_request;

  SCM name;
  SCM update_func;
} ngx_http_guile_request_t;

/* Constructors */

SCM ngx_http_guile_request_c_make (char *name, ngx_http_request_t *r);

/* Initialization */

void ngx_http_guile_init_req_foreign_type ();

/* Accessors */

SCM ngx_http_guile_request_http_version (SCM http_request);
SCM ngx_http_guile_request_http_protocol (SCM http_request);
SCM ngx_http_guile_request_request_line (SCM http_request);
SCM ngx_http_guile_request_method (SCM http_request);
SCM ngx_http_guile_request_uri (SCM http_request);
SCM ngx_http_guile_request_args (SCM http_request);
SCM ngx_http_guile_request_exten (SCM http_request);
SCM ngx_http_guile_request_unparsed_uri (SCM http_request);

#endif
