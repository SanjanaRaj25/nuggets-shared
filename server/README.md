# Server

This subdirectory contains the server side of the Nuggest game, which is the core module of this project. It draws on the helper modules hosted within the common, libs, support, and maps subdirectories.
The files included are:

* `server.c`: implementation of the main (server) module
* `Makefile`: builds server

## Compilation

To compile,

	make server.c

To clean,

	make clean