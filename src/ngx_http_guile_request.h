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

SCM ngx_http_guile_request_header_in (SCM http_request, SCM header_name);

SCM ngx_http_guile_request_header_host (SCM http_request);
SCM ngx_http_guile_request_header_connection (SCM http_request);
SCM ngx_http_guile_request_header_if_modified_since (SCM http_request);
SCM ngx_http_guile_request_header_if_unmodified_since (SCM http_request);
SCM ngx_http_guile_request_header_if_match (SCM http_request);
SCM ngx_http_guile_request_header_if_none_match (SCM http_request);
SCM ngx_http_guile_request_header_user_agent (SCM http_request);
SCM ngx_http_guile_request_header_referer (SCM http_request);
SCM ngx_http_guile_request_header_content_length (SCM http_request);
SCM ngx_http_guile_request_header_content_range (SCM http_request);
SCM ngx_http_guile_request_header_content_type (SCM http_request);
SCM ngx_http_guile_request_header_range (SCM http_request);
SCM ngx_http_guile_request_header_if_range (SCM http_request);
SCM ngx_http_guile_request_header_transfer_encoding (SCM http_request);
SCM ngx_http_guile_request_header_te (SCM http_request);
SCM ngx_http_guile_request_header_expect (SCM http_request);
SCM ngx_http_guile_request_header_upgrade (SCM http_request);

#if (NGX_HTTP_GZIP || NGX_HTTP_HEADERS)
SCM ngx_http_guile_request_header_accept_encoding (SCM http_request);
SCM ngx_http_guile_request_header_via (SCM http_request);
#endif

SCM ngx_http_guile_request_header_authorization (SCM http_request);
SCM ngx_http_guile_request_header_keep_alive (SCM http_request);

#if (NGX_HTTP_X_FORWARDED_FOR)
SCM ngx_http_guile_request_header_x_forwarded_for (SCM http_request);
#endif

#if (NGX_HTTP_REALIP)
SCM ngx_http_guile_request_header_x_real_ip (SCM http_request);
#endif

#if (NGX_HTTP_HEADERS)
SCM ngx_http_guile_request_header_accept (SCM http_request);
SCM ngx_http_guile_request_header_accept_language (SCM http_request);
#endif

#if (NGX_HTTP_DAV)
SCM ngx_http_guile_request_header_depth (SCM http_request);
SCM ngx_http_guile_request_header_destination (SCM http_request);
SCM ngx_http_guile_request_header_overwrite (SCM http_request);
SCM ngx_http_guile_request_header_date (SCM http_request);
#endif

SCM ngx_http_guile_request_header_cookie (SCM http_request);

SCM ngx_http_guile_request_user (SCM http_request);
SCM ngx_http_guile_request_passwd (SCM http_request);

#endif /* _NGX_HTTP_GUILE_REQUEST_INCLUDED_ */
