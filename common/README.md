# Common

This subdirectory contains the `game` and `grid` helper modules, which are used by the main module, `server`.
The files included are:

* `game.c`: implementation of module handling high level game properties
* `game.h`: interface of module handling high level game properties
* `grid.c`: implementation of module handling grid initialization, updating, and display
* `grid.h`: interface of module handling grid initialization, updating, and display
* `structs.h`: defines 3 custom structures employed by `game`, `grid`, and `server`
* `Makefile`: builds common.a

## Compilation

To compile,

	make common.a

To clean,

	make clean