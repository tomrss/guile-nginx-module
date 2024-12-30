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
#include "ngx_http_guile_request.h"

// TODO dangerous global, move it in configuration scope
static SCM ngx_http_guile_request_scm;

/* Local helpers */

static ngx_http_request_t *unwrap_http_request (SCM http_request);
static SCM scm_from_ngx_string (ngx_str_t str);
static ngx_table_elt_t *search_hashed_headers_in (ngx_http_request_t *r,
                                                  u_char *name, size_t len);

/* Initializations */

void
ngx_http_guile_init_req_foreign_type ()
{
  SCM name, slots;
  scm_t_struct_finalize finalizer;

  name = scm_from_utf8_symbol ("ngx-http-request");

  slots = scm_list_1 (scm_from_utf8_symbol ("http-request"));

  // TODO
  finalizer = NULL;

  ngx_http_guile_request_scm
      = scm_make_foreign_object_type (name, slots, finalizer);
}

/* Constructors */

SCM
ngx_http_guile_request_c_make (char *name, ngx_http_request_t *r)
{
  ngx_http_guile_request_t *req_scm;

  req_scm
      = scm_gc_malloc (sizeof (ngx_http_guile_request_t), "ngx-http-request");

  req_scm->name = scm_from_utf8_symbol (name);
  req_scm->http_request = r;

  return scm_make_foreign_object_1 (ngx_http_guile_request_scm, req_scm);
}

/* Accessors */

SCM
ngx_http_guile_request_http_version (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ulong (r->http_version);
}

SCM
ngx_http_guile_request_http_protocol (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->http_protocol);
}

SCM
ngx_http_guile_request_request_line (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->request_line);
}

SCM
ngx_http_guile_request_method (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  char *method;
  switch (r->method)
    {
    // TODO other methods
    case NGX_HTTP_GET:
      method = "GET";
      break;
    case NGX_HTTP_POST:
      method = "POST";
      break;
    case NGX_HTTP_PUT:
      method = "PUT";
      break;
    case NGX_HTTP_DELETE:
      method = "DELETE";
      break;
    default:
      // TODO
      method = "GET";
    }

  return scm_from_utf8_string (method);
}

SCM
ngx_http_guile_request_uri (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->uri);
}

SCM
ngx_http_guile_request_args (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->args);
}

SCM
ngx_http_guile_request_exten (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->exten);
}

SCM
ngx_http_guile_request_unparsed_uri (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->unparsed_uri);
}

SCM
ngx_http_guile_request_header_in (SCM http_request, SCM header_name)
{
  ngx_http_request_t *r;
  char *header_key;
  ngx_table_elt_t *header;

  r = unwrap_http_request (http_request);
  header_key = scm_to_locale_string (header_name);
  header = search_hashed_headers_in (r, (u_char *)header_key,
                                     ngx_strlen (header_key));

  return scm_from_ngx_string (header->value);
}

SCM
ngx_http_guile_request_header_host (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.host->value);
}

SCM
ngx_http_guile_request_header_connection (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.connection->value);
}

SCM
ngx_http_guile_request_header_if_modified_since (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.if_modified_since->value);
}

SCM
ngx_http_guile_request_header_if_unmodified_since (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.if_unmodified_since->value);
}

SCM
ngx_http_guile_request_header_if_match (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.if_match->value);
}

SCM
ngx_http_guile_request_header_if_none_match (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.if_none_match->value);
}

SCM
ngx_http_guile_request_header_user_agent (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.user_agent->value);
}

SCM
ngx_http_guile_request_header_referer (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.referer->value);
}

SCM
ngx_http_guile_request_header_content_length (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.content_length->value);
}

SCM
ngx_http_guile_request_header_content_range (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.content_range->value);
}

SCM
ngx_http_guile_request_header_content_type (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.content_type->value);
}

SCM
ngx_http_guile_request_header_range (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.range->value);
}

SCM
ngx_http_guile_request_header_if_range (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.if_range->value);
}

SCM
ngx_http_guile_request_header_transfer_encoding (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.transfer_encoding->value);
}

SCM
ngx_http_guile_request_header_te (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.te->value);
}

SCM
ngx_http_guile_request_header_expect (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.expect->value);
}

SCM
ngx_http_guile_request_header_upgrade (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.upgrade->value);
}

#if (NGX_HTTP_GZIP || NGX_HTTP_HEADERS)

SCM
ngx_http_guile_request_header_accept_encoding (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.accept_encoding->value);
}

SCM
ngx_http_guile_request_header_via (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.via->value);
}

#endif

SCM
ngx_http_guile_request_header_authorization (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.authorization->value);
}

SCM
ngx_http_guile_request_header_keep_alive (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.keep_alive->value);
}

#if (NGX_HTTP_X_FORWARDED_FOR)

SCM
ngx_http_guile_request_header_x_forwarded_for (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.x_forwarded_for->value);
}

#endif

#if (NGX_HTTP_REALIP)

SCM ngx_http_guile_request_header_x_real_ip (SCM http_request);

#endif

#if (NGX_HTTP_HEADERS)

SCM
ngx_http_guile_request_header_accept (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.accept->value);
}

SCM
ngx_http_guile_request_header_accept_language (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.accept_language->value);
}

#endif

#if (NGX_HTTP_DAV)

SCM
ngx_http_guile_request_header_depth (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.depth->value);
}

SCM
ngx_http_guile_request_header_destination (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.destination->value);
}

SCM
ngx_http_guile_request_header_overwrite (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.overwrite->value);
}

SCM
ngx_http_guile_request_header_date (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.date->value);
}

#endif

SCM
ngx_http_guile_request_header_cookie (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.cookie->value);
}

SCM
ngx_http_guile_request_user (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.user);
}

SCM
ngx_http_guile_request_passwd (SCM http_request)
{
  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->headers_in.passwd);
}

/* Local helpers impl */

static ngx_http_request_t *
unwrap_http_request (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_guile_request_t *r = scm_foreign_object_ref (http_request, 0);

  return r->http_request;
}

static SCM
scm_from_ngx_string (ngx_str_t str)
{
  return scm_from_locale_stringn ((char *)str.data, str.len);
}

/* Function copied from here:
   https://www.nginx.com/resources/wiki/start/topics/examples/headers_management/#quick-search-with-hash
*/
static ngx_table_elt_t *
search_hashed_headers_in (ngx_http_request_t *r, u_char *name, size_t len)
{
  ngx_http_core_main_conf_t *cmcf;
  ngx_http_header_t *hh;
  u_char *lowcase_key;
  ngx_uint_t i, hash;

  /*
  Header names are case-insensitive, so have been hashed by lowercases key
  */
  lowcase_key = ngx_palloc (r->pool, len);
  if (lowcase_key == NULL)
    return NULL;

  /*
  Calculate a hash of lowercased header name
  */
  hash = 0;
  for (i = 0; i < len; i++)
    {
      lowcase_key[i] = ngx_tolower (name[i]);
      hash = ngx_hash (hash, lowcase_key[i]);
    }

  /*
  The layout of hashed headers is stored in ngx_http_core_module main config.
  All the hashes, its offsets and handlers are pre-calculated
  at the configuration time in ngx_http_init_headers_in_hash() at
  ngx_http.c:432 with data from ngx_http_headers_in at ngx_http_request.c:80.
  */
  cmcf = ngx_http_get_module_main_conf (r, ngx_http_core_module);

  /*
  Find the current header description (ngx_http_header_t) by its hash
  */
  hh = ngx_hash_find (&cmcf->headers_in_hash, hash, lowcase_key, len);

  if (hh == NULL)
    /*
    There header is unknown or is not hashed yet.
    */
    return NULL;

  if (hh->offset == 0)
    /*
    There header is hashed but not cached yet for some reason.
    */
    return NULL;

  /*
  The header value was already cached in some field
  of the r->headers_in struct (hh->offset tells in which one).
  */

  return *((ngx_table_elt_t **)((char *)&r->headers_in + hh->offset));
}
