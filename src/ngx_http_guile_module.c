#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_crypt.h>
#include <ngx_http.h>
// has to be included after ngx
#include "ngx_http_guile_request.h"
#include <libguile.h>
#include <time.h>

// TODO dangerous global, move it in config scope
static SCM parse_req_function;

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
  ngx_http_request_t *http_request = (ngx_http_request_t *)data;

  // TODO unique request name
  SCM http_request_scm
      = ngx_http_guile_request_c_make ("my-req", http_request);

  scm_call_1 (scm_variable_ref (parse_req_function), http_request_scm);

  return NULL;
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

  // load the script
  scm_primitive_load (scm_from_locale_stringn ((char *)script_filename->data,
                                               script_filename->len));

  // cache script variables
  SCM handler_proc = scm_from_utf8_symbol ("ngx-handle-request");
  parse_req_function = scm_module_lookup (scm_current_module (), handler_proc);

  return NULL;
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
  scm_with_guile (ngx_http_guile_init_scm, glcf->init_script);

  return NGX_CONF_OK;
}
