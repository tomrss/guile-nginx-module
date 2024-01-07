#ifndef _NGX_HTTP_GUILE_REQUEST_INCLUDED_
#define _NGX_HTTP_GUILE_REQUEST_INCLUDED_

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
