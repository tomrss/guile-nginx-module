# Guile NGINX module

Module for enabling NGINX extensions in GNU Guile Scheme.

The goal is to be able to extend NGINX in a interpreted and hackable 
language.
Guile Scheme is chosen as standard extension language of the GNU project.

## Status

**WARNING**: this module is under early stages of development and it is at the
moment not usable in any practical case.

Don't use it in any case except for contributing or for experimenting.

## Installation

Requirements:
- nginx sources
- libguile 3.0

For using this module in nginx, you must compile nginx with a reference to this
module source. You can find instruction below also in [nginx docs about 
compiling modules](https://www.nginx.com/resources/wiki/extending/compiling/).

Download nginx source code and, in nginx sources root, run configure to generate
makefile:

```shell
./configure --add-module=<path/to/this/project>
```

If using GCC, run:
```shell
./configure --with-cc=gcc --add-module=<path/to/this/project>
```

Then compile with:
```shell
make
```

## Development

You can generate a `compile_commands.json` for usage in common language servers
like `ccls` for example using [bear](https://github.com/rizsotto/Bear).
Run `.configure` as shown above in nginx sources and then run in this folder:

```shell
bear -- make -C <path/to/nginx/sources/root>
```

## Writing Scheme extensions

TODO

## Contributing

Any type of contribution is welcome.
