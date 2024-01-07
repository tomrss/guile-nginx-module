#include "ngx_http_guile_request.h"

// TODO dangerous global, move it in configuration scope
static SCM ngx_http_guile_request_scm;

/* Local helpers */

static ngx_http_request_t *unwrap_http_request (SCM http_request);
static SCM scm_from_ngx_string (ngx_str_t str);

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

  req_scm = (ngx_http_guile_request_t *)scm_gc_malloc (
      sizeof (ngx_http_guile_request_t), "ngx-http-request");

  req_scm->name = scm_from_utf8_symbol (name);
  req_scm->http_request = r;

  return scm_make_foreign_object_1 (ngx_http_guile_request_scm, req_scm);
}

/* Accessors */

SCM
ngx_http_guile_request_http_version (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ulong (r->http_version);
}

SCM
ngx_http_guile_request_http_protocol (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->http_protocol);
}

SCM
ngx_http_guile_request_request_line (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->request_line);
}

SCM
ngx_http_guile_request_method (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

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
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->uri);
}

SCM
ngx_http_guile_request_args (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->args);
}

SCM
ngx_http_guile_request_exten (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->exten);
}

SCM
ngx_http_guile_request_unparsed_uri (SCM http_request)
{
  scm_assert_foreign_object_type (ngx_http_guile_request_scm, http_request);

  ngx_http_request_t *r = unwrap_http_request (http_request);

  return scm_from_ngx_string (r->unparsed_uri);
}

/* Local helpers impl */

static ngx_http_request_t *
unwrap_http_request (SCM http_request)
{
  ngx_http_guile_request_t *r
      = (ngx_http_guile_request_t *)scm_foreign_object_ref (http_request, 0);
  return r->http_request;
}

static SCM
scm_from_ngx_string (ngx_str_t str)
{
  return scm_from_locale_stringn ((char *)str.data, str.len);
}
