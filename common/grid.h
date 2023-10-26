/*
Header files for grid.c, set of functions to be used by the game and server modules.
Team 9: Plankton, May 2023
*/

#ifndef __GRID_H_
#define __GRID_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../libs/file.h"
#include "../libs/mem.h"

#include "../support/log.h"
#include "../support/message.h"

#include "structs.h"
#include "game.h"


/*
 * load_grid
 * Loads the game grid from a file. 
 * Specifically, reads a file into an array of strings, with each string in the array representing a row of the game map.
 * Inputs:
 *   - fp: Pointer to the file containing the grid.
 *   - rows: Pointer to the variable that will store the number of rows in the grid.
 *   - columns: Pointer to the variable that will store the number of columns in the grid.
 * Outputs:
 *   - Returns a dynamically allocated 2D array representing the game grid.
 * Notes: caller must later call grid_delete
 */
char** load_grid(FILE* fp, int* rows, int* columns);

/*
 * load_player_grid
 * Loads the player's grid based on the game state.
 * The player's grid starts off empty and has an equal rows and columns as the global grid.
 * Inputs:
 *   - game: Pointer to the game state structure.
 * Outputs:
 *   - Returns a dynamically allocated 2D array representing the player's grid.
 * Notes: caller must later call grid_delete
 */
char** load_player_grid(game_t* game);

/*
 * grid_toStr
 * Converts the game grids to a string representation.
 * Inputs:
 *   - global_grid: Pointer to the global grid array.
 *   - player_grid: Pointer to the player's grid array.
 *   - rows: Number of rows in the grids.
 *   - columns: Number of columns in the grids.
 * Outputs:
 *   - Returns a dynamically allocated string representing the grids.
 */
char* grid_toStr(char** global_grid, char** player_grid, int rows, int columns);

/*
 * assign_random_spot
 * Assigns a random spot in the grid for a given object.
 * Used to randomly place players and gold.
 * Inputs:
 *   - grid: Pointer to the game grid array.
 *   - rows: Number of rows in the grid.
 *   - columns: Number of columns in the grid.
 *   - thing: Character representing the thing to assign.
 *   - spot_r: Pointer to the variable that will store the assigned row.
 *   - spot_c: Pointer to the variable that will store the assigned column.
 * Outputs: None
 */
void assign_random_spot(char** grid, int rows, int columns, char thing, int* spot_x, int* spot_y);


/*
 * get_grid_value
 * Retrieves the value/symbol at a specific position in the game grid.
 * Inputs:
 *   - game: Pointer to the game state structure.
 *   - r: Row index in the grid.
 *   - c: Column index in the grid.
 * Outputs:
 *   - Returns the value at the specified position in the grid.
 */
char get_grid_value(game_t* game, int r, int c);


/*
 * change_spot
 * Changes the value/symbol at a specific position in the game grid.
 * Inputs:
 *   - game: Pointer to the game state structure.
 *   - r: Row index in the grid.
 *   - c: Column index in the grid.
 *   - symbol: Character representing the new value to assign.
 * Outputs: None
 */
void change_spot(game_t* game, int r, int c, char symbol);


/*
 * is_visible
 * Checks if a position in the grid is visible from the player's current location.
 * Checks spots on each row and column between the player and the position investigated for boundaries that might obstruct visibility.
 * Inputs:
 *   - game: Pointer to the game state structure.
 *   - playerColumn: Column index of the player's position.
 *   - playerRow: Row index of the player's position.
 *   - column: Column index of the position to check.
 *   - row: Row index of the position to check.
 * Outputs:
 *   - Returns true if the position is visible to the player, false otherwise.
 */
bool is_visible(game_t* game, const int playerColumn, const int playerRow, const int column, const int row);

/*
 * get_player_visible
 * Retrieves the visibility status of a player.
 * Loops over each point in the grid and calls `is_visible`, updating the player's grid accordingly. 
 * Returns true if the player's grid was modified.
 * Inputs:
 *   - game: Pointer to the game state structure.
 *   - player: Pointer to the player structure.
 * Outputs:
 *   - Returns true if the player is visible, false otherwise.
 */
bool get_player_visible(game_t* game, client_t* player);

/*
 * grid_delete
 * Deallocates the memory occupied by the game grid.
 * Inputs:
 *   - grid: Pointer to the game grid array.
 *   - rows: Number of rows in the grid.
 * Outputs: None
 */
void grid_delete(char** grid, int rows);


#endif // __GRID_H_