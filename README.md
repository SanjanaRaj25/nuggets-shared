# Nuggets
### Team 9 - Plankton: Jackson Easley, Andrada Pantelimon, Sanjana Raj

This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game. To play with our team's map, load *plankton.txt*.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.

Assumptions:
Game over message doesn't include info about quit players and only gives the scores of players present when the game ends.

## Limitations
Our code begins to lag and run slowly with large numbers of bots and despit our efforts to increase the efficiency of visibility and sending messages only when necessary, large numbers of bots begin to slow down the program. 

If we had more time / more people writing code we would have tried to be more thorough in handling larger games without slowing down and could have tested out more potential fixes.

## Subdirectories

The subdirectories included are:
* [server](server/README.md): contains the server side of Nuggets, which is the core module of this project
* [common](common/README.md): contains the `game` and `grid` helper modules
* [support](support/README.md): contains useful modules provided by the instructor
* [maps](maps/README.md): contains our team's map, `plankton.txt`, and some draft maps

The high level files included are:
* `DESIGN.md`: describes the design spec of our project
* `IMPLEMENTATION.md`: describes the implementation plan for our project
* `Makefile`: compiles all programs
* `.gitignore`: describes files to be ignored by git

## Compilation

To compile,

	make all

To clean,

	make clean
