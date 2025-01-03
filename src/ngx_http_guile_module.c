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
#include <ngx_crypt.h>
#include <ngx_http.h>
// has to be included after ngx
#include "ngx_http_guile_request.h"
#include <libguile.h>
#include <time.h>

#define NGX_HTTP_GUILE_MODULE "ngx http base"

typedef struct
{
  ngx_http_complex_value_t *init_script;
} ngx_http_guile_loc_conf_t;

static ngx_int_t ngx_http_guile_handler (ngx_http_request_t *r);
static void *ngx_http_guile_create_loc_conf (ngx_conf_t *cf);
static char *ngx_http_guile_merge_loc_conf (ngx_conf_t *cf, void *parent,
                                            void *child);
static ngx_int_t ngx_http_guile_init (ngx_conf_t *cf);
static char *ngx_http_guile_init_script (ngx_conf_t *cf, ngx_command_t *cmd,
                                         void *conf);
static void *ngx_http_guile_init_scm (void *data);
static void ngx_http_guile_init_module (void *data);
static void *ngx_http_guile_handle_request (void *data);
static SCM ngx_http_guile_handle_request_in_module (void *data);

static ngx_command_t ngx_http_guile_commands[] = {

  { ngx_string ("guile_init_script"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF
        | NGX_HTTP_LMT_CONF | NGX_CONF_TAKE1,
    ngx_http_guile_init_script, NGX_HTTP_LOC_CONF_OFFSET,
    offsetof (ngx_http_guile_loc_conf_t, init_script), NULL },

  ngx_null_command
};

static ngx_http_module_t ngx_http_guile_module_ctx = {
  NULL,                /* preconfiguration */
  ngx_http_guile_init, /* postconfiguration */

  NULL, /* create main configuration */
  NULL, /* init main configuration */

  NULL, /* create server configuration */
  NULL, /* merge server configuration */

  ngx_http_guile_create_loc_conf, /* create location configuration */
  ngx_http_guile_merge_loc_conf   /* merge location configuration */
};

ngx_module_t ngx_http_guile_module
    = { NGX_MODULE_V1,
        &ngx_http_guile_module_ctx, /* module context */
        ngx_http_guile_commands,    /* module directives */
        NGX_HTTP_MODULE,            /* module type */
        NULL,                       /* init master */
        NULL,                       /* init module */
        NULL,                       /* init process */
        NULL,                       /* init thread */
        NULL,                       /* exit thread */
        NULL,                       /* exit process */
        NULL,                       /* exit master */
        NGX_MODULE_V1_PADDING };

static void *
ngx_http_guile_handle_request (void *data)
{
  SCM module = scm_c_resolve_module (NGX_HTTP_GUILE_MODULE);
  scm_c_call_with_current_module (
      module, ngx_http_guile_handle_request_in_module, data);

  return NULL;
}

static SCM
ngx_http_guile_handle_request_in_module (void *data)
{
  ngx_http_request_t *http_request = data;

  // TODO unique request name
  SCM http_request_scm
      = ngx_http_guile_request_c_make ("my-req", http_request);

  SCM parse_request_fun = scm_c_lookup ("ngx-handle-request");

  scm_call_1 (scm_variable_ref (parse_request_fun), http_request_scm);

  return http_request_scm;
}

static ngx_int_t
ngx_http_guile_handler (ngx_http_request_t *r)
{
  scm_with_guile (&ngx_http_guile_handle_request, r);

  return NGX_OK;
}

static void *
ngx_http_guile_create_loc_conf (ngx_conf_t *cf)
{
  ngx_http_guile_loc_conf_t *conf;

  conf = ngx_pcalloc (cf->pool, sizeof (ngx_http_guile_loc_conf_t));
  if (conf == NULL)
    return NULL;

  conf->init_script = NGX_CONF_UNSET_PTR;

  return conf;
}

static char *
ngx_http_guile_merge_loc_conf (ngx_conf_t *cf, void *parent, void *child)
{
  ngx_http_guile_loc_conf_t *prev = parent;
  ngx_http_guile_loc_conf_t *conf = child;

  ngx_conf_merge_ptr_value (conf->init_script, prev->init_script, NULL);

  return NGX_CONF_OK;
}

static void *
ngx_http_guile_init_scm (void *data)
{
  scm_c_define_module (NGX_HTTP_GUILE_MODULE, ngx_http_guile_init_module,
                       data);
  ngx_http_guile_init_module (data);

  return NULL;
}

static void
ngx_http_guile_init_module (void *data)
{

  ngx_str_t *script_filename = (ngx_str_t *)data;

  // initialize data types
  ngx_http_guile_init_req_foreign_type ();

  // register functions
  // TODO other
  scm_c_define_gsubr ("ngx-request-http-version", 1, 0, 0,
                      ngx_http_guile_request_http_version);
  scm_c_define_gsubr ("ngx-request-http-protocol", 1, 0, 0,
                      ngx_http_guile_request_http_protocol);
  scm_c_define_gsubr ("ngx-request-request-line", 1, 0, 0,
                      ngx_http_guile_request_request_line);
  scm_c_define_gsubr ("ngx-request-method", 1, 0, 0,
                      ngx_http_guile_request_method);
  scm_c_define_gsubr ("ngx-request-uri", 1, 0, 0, ngx_http_guile_request_uri);
  scm_c_define_gsubr ("ngx-request-args", 1, 0, 0,
                      ngx_http_guile_request_args);
  scm_c_define_gsubr ("ngx-request-exten", 1, 0, 0,
                      ngx_http_guile_request_exten);
  scm_c_define_gsubr ("ngx-request-unparsed-uri", 1, 0, 0,
                      ngx_http_guile_request_unparsed_uri);

  scm_c_define_gsubr ("ngx-request-header-in", 2, 0, 0,
                      ngx_http_guile_request_header_in);

  scm_c_define_gsubr ("ngx-request-header-host", 1, 0, 0,
                      ngx_http_guile_request_header_host);
  scm_c_define_gsubr ("ngx-request-header-connection", 1, 0, 0,
                      ngx_http_guile_request_header_connection);
  scm_c_define_gsubr ("ngx-request-header-if-modified-since", 1, 0, 0,
                      ngx_http_guile_request_header_if_modified_since);
  scm_c_define_gsubr ("ngx-request-header-if-unmodified-since", 1, 0, 0,
                      ngx_http_guile_request_header_if_unmodified_since);
  scm_c_define_gsubr ("ngx-request-header-if-match", 1, 0, 0,
                      ngx_http_guile_request_header_if_match);
  scm_c_define_gsubr ("ngx-request-header-if-none-match", 1, 0, 0,
                      ngx_http_guile_request_header_if_none_match);
  scm_c_define_gsubr ("ngx-request-header-user-agent", 1, 0, 0,
                      ngx_http_guile_request_header_user_agent);
  scm_c_define_gsubr ("ngx-request-header-referer", 1, 0, 0,
                      ngx_http_guile_request_header_referer);
  scm_c_define_gsubr ("ngx-request-header-content-length", 1, 0, 0,
                      ngx_http_guile_request_header_content_length);
  scm_c_define_gsubr ("ngx-request-header-content-range", 1, 0, 0,
                      ngx_http_guile_request_header_content_range);
  scm_c_define_gsubr ("ngx-request-header-content-type", 1, 0, 0,
                      ngx_http_guile_request_header_content_type);
  scm_c_define_gsubr ("ngx-request-header-range", 1, 0, 0,
                      ngx_http_guile_request_header_range);
  scm_c_define_gsubr ("ngx-request-header-if-range", 1, 0, 0,
                      ngx_http_guile_request_header_if_range);
  scm_c_define_gsubr ("ngx-request-header-transfer-encoding", 1, 0, 0,
                      ngx_http_guile_request_header_transfer_encoding);
  scm_c_define_gsubr ("ngx-request-header-te", 1, 0, 0,
                      ngx_http_guile_request_header_te);
  scm_c_define_gsubr ("ngx-request-header-expect", 1, 0, 0,
                      ngx_http_guile_request_header_expect);
  scm_c_define_gsubr ("ngx-request-header-upgrade", 1, 0, 0,
                      ngx_http_guile_request_header_upgrade);

#if (NGX_HTTP_GZIP || NGX_HTTP_HEADERS)
  scm_c_define_gsubr ("ngx-request-header-accept-encoding", 1, 0, 0,
                      ngx_http_guile_request_header_accept_encoding);
  scm_c_define_gsubr ("ngx-request-header-via", 1, 0, 0,
                      ngx_http_guile_request_header_via);
#endif

  scm_c_define_gsubr ("ngx-request-header-authorization", 1, 0, 0,
                      ngx_http_guile_request_header_authorization);
  scm_c_define_gsubr ("ngx-request-header-keep-alive", 1, 0, 0,
                      ngx_http_guile_request_header_keep_alive);

#if (NGX_HTTP_X_FORWARDED_FOR)
  scm_c_define_gsubr ("ngx-request-header-x-forwarded-for", 1, 0, 0,
                      ngx_http_guile_request_header_x_forwarded_for);
#endif

#if (NGX_HTTP_REALIP)
  scm_c_define_gsubr ("ngx-request-header-x-real-ip", 1, 0, 0,
                      ngx_http_guile_request_header_x_real_ip);
#endif

#if (NGX_HTTP_HEADERS)
  scm_c_define_gsubr ("ngx-request-header-accept", 1, 0, 0,
                      ngx_http_guile_request_header_accept);
  scm_c_define_gsubr ("ngx-request-header-accept-language", 1, 0, 0,
                      ngx_http_guile_request_header_accept_language);
#endif

#if (NGX_HTTP_DAV)
  scm_c_define_gsubr ("ngx-request-header-depth", 1, 0, 0,
                      ngx_http_guile_request_header_depth);
  scm_c_define_gsubr ("ngx-request-header-destination", 1, 0, 0,
                      ngx_http_guile_request_header_destination);
  scm_c_define_gsubr ("ngx-request-header-overwrite", 1, 0, 0,
                      ngx_http_guile_request_header_overwrite);
  scm_c_define_gsubr ("ngx-request-header-date", 1, 0, 0,
                      ngx_http_guile_request_header_date);
#endif

  scm_c_define_gsubr ("ngx-request-header-cookie", 1, 0, 0,
                      ngx_http_guile_request_header_cookie);

  scm_c_define_gsubr ("ngx-request-user", 1, 0, 0,
                      ngx_http_guile_request_user);
  scm_c_define_gsubr ("ngx-request-passwd", 1, 0, 0,
                      ngx_http_guile_request_passwd);

  // export functions in current module
  scm_c_export (
      "ngx-request-http-version", "ngx-request-http-protocol",
      "ngx-request-request-line", "ngx-request-method", "ngx-request-uri",
      "ngx-request-args", "ngx-request-exten", "ngx-request-unparsed-uri",
      "ngx-request-header-in", "ngx-request-header-host",
      "ngx-request-header-connection", "ngx-request-header-if-modified-since",
      "ngx-request-header-if-unmodified-since", "ngx-request-header-if-match",
      "ngx-request-header-if-none-match", "ngx-request-header-user-agent",
      "ngx-request-header-referer", "ngx-request-header-content-length",
      "ngx-request-header-content-range", "ngx-request-header-content-type",
      "ngx-request-header-range", "ngx-request-header-if-range",
      "ngx-request-header-transfer-encoding", "ngx-request-header-te",
      "ngx-request-header-expect", "ngx-request-header-upgrade",
#if (NGX_HTTP_GZIP || NGX_HTTP_HEADERS)
      "ngx-request-header-accept-encoding", "ngx-request-header-via",
#endif
      "ngx-request-header-authorization", "ngx-request-header-keep-alive",
#if (NGX_HTTP_X_FORWARDED_FOR)
      "ngx-request-header-x-forwarded-for",
#endif
#if (NGX_HTTP_REALIP)
      "ngx-request-header-x-real-ip",
#endif
#if (NGX_HTTP_HEADERS)
      "ngx-request-header-accept", "ngx-request-header-accept-language",
#endif
#if (NGX_HTTP_DAV)
      "ngx-request-header-depth", "ngx-request-header-destination",
      "ngx-request-header-overwrite", "ngx-request-header-date",
#endif
      "ngx-request-header-cookie", "ngx-request-user", "ngx-request-passwd",
      NULL);

  // load the script
  scm_primitive_load (scm_from_locale_stringn ((char *)script_filename->data,
                                               script_filename->len));
}

static ngx_int_t
ngx_http_guile_init (ngx_conf_t *cf)
{
  ngx_http_handler_pt *h;
  ngx_http_core_main_conf_t *cmcf;

  cmcf = ngx_http_conf_get_module_main_conf (cf, ngx_http_core_module);

  // register handler
  h = ngx_array_push (&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
  if (h == NULL)
    return NGX_ERROR;

  *h = ngx_http_guile_handler;

  return NGX_OK;
}

static char *
ngx_http_guile_init_script (ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_guile_loc_conf_t *glcf = conf;
  ngx_http_compile_complex_value_t ccv;
  ngx_str_t *script_path;

  if (glcf->init_script != NGX_CONF_UNSET_PTR)
    return "is duplicate";

  script_path = cf->args->elts;

  glcf->init_script = ngx_palloc (cf->pool, sizeof (ngx_http_complex_value_t));
  if (glcf->init_script == NULL)
    return NGX_CONF_ERROR;

  script_path = cf->args->elts;

  ngx_memzero (&ccv, sizeof (ngx_http_compile_complex_value_t));

  ccv.cf = cf;
  ccv.value = &script_path[1];
  ccv.complex_value = glcf->init_script;
  ccv.zero = 1;
  ccv.conf_prefix = 1;

  if (ngx_http_compile_complex_value (&ccv) != NGX_OK)
    return NGX_CONF_ERROR;

  // init guile
  scm_with_guile (ngx_http_guile_init_scm, ccv.value);

  return NGX_CONF_OK;
}
