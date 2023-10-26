/*
grid.c 
module for handling the global and player grids
specific function descriptions are located in grid.h
Team 9: Plankton, May 2023
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "../libs/file.h"
#include "../libs/mem.h"

#include "../support/log.h"
#include "../support/message.h"

#include "structs.h"
#include "game.h"
#include "grid.h"

/**************** static function declarations  ****************/

/*
* is_open: takes in game, column, and row, and returns true if the spot is one where a player can move to
*/
static bool is_open(game_t* game, const int c, const int r);

/*
* is_integer: takes in a value, determines whether it's an integer, returns boolean
*/
static bool is_integer(float num);

/*
* is_gridspot: takes in two doubles, returns true if they're both ints (otherwise, need to check top and bottom)
*/
static bool is_gridspot(double a, double b);


/**************** local function declarations  ****************/

/**************** load_grid  ****************/
char** 
load_grid(FILE* fp, int* rows, int* columns)
{
    // defensive check: file isn't null
    if (fp == NULL){
        fprintf(stderr, "Error. NULL file pointer passed to load_map.\n");
        exit(1);
    }

    // Create an array of strings, each string is one row of the map
    *rows = file_numLines(fp);
    char** grid = mem_malloc_assert(*rows * sizeof(char*), "Error allocating memory in load_grid.\n");
    char* newRow = NULL;
    int row = 0; // keeps track of our position while filling in grid array

    char* line = file_readLine(fp);
    *columns = strlen(line);
    mem_free(line); // responsible for freeing this memory

    rewind(fp);  // reset pointer to beginning of the file

    while ((newRow = file_readLine(fp)) != NULL){
        grid[row] = newRow;
        row++;
    }
    // this is a 2D character array

    return grid;
}

/**************** load_player_grid  ****************/
char**
load_player_grid(game_t* game)
{
    char** grid = mem_malloc_assert(game->rows * sizeof(char*), "Error allocating memory in load_player_grid.\n");
    

    char* emptyRow = mem_malloc_assert(game->columns + 1, "Error allocating memory in load_player_grid.\n");

    for (int c = 0; c < game->columns; c++){
        emptyRow[c] = ' ';
    }

    emptyRow[game->columns] = '\0';


    for (int r = 0; r < game->rows; r++){
        grid[r] = mem_malloc_assert(game->columns + 1, "Error allocating memory in load_player_grid.\n");
        strcpy(grid[r], emptyRow);
    }

    mem_free(emptyRow);

    return grid;
}

/**************** grid_toStr  ****************/
char*
grid_toStr(char** global_grid, char** player_grid, int rows, int columns)
{
   // Create string for string version of grid map, must have rows*columns characters plus new lines & a terminating null
   char* display = mem_malloc_assert((rows* (columns + 1)) , "Error allocating memory in grid_toStr.\n");

   for (int r = 0; r < rows; r++){
        for (int c = 0; c < columns; c++){
            // adding 1 because new line isn't included in column count
            display[(r * (columns + 1)) + c] = global_grid[r][c];
           
            if (player_grid != NULL){
                // if a player grid was passed in, set to player_grid value
                display[(r* (columns + 1)) + c] = player_grid[r][c];
            }
        }

        display[(r* (columns + 1)) + columns] = '\n'; // add new line to the end of each row
    
    }

    display[(rows*columns) + rows - 1] = '\0'; // null terminate string

    // this is a string, in a format that can be sent directly to the client 
   return display;
}

/**************** assign_random_spot  ****************/
void
assign_random_spot(char** grid, int rows, int columns, char thing, int* spot_r, int* spot_c)
{
    // assigns a "thing" to a random open spot, can be used to place either gold or a player
    bool placed = false;
    int r;
    int c;
    srand(time(NULL));

    while (!placed){
        // get a pseudo-random x coordinate and y coordinate from stdlib

        r = rand() % rows;
        c = rand() % columns;

        // try to place the "thing" there

        char value = grid[r][c];

        if (value == '.'){

            grid[r][c] = thing;
            placed = true;

            // assign spot x and y
            *spot_r = r;
            *spot_c = c;
        }
        // try again with another random spot if it didn't work
    }
}


/**************** get_grid_value  ****************/
char 
get_grid_value(game_t* game, int r, int c)
{
    return game->grid[r][c];
}


/**************** change_spot  ****************/
void 
change_spot(game_t* game, int r, int c, char symbol)
{
    game->grid[r][c] = symbol;
}

/**************** is_open ****************/
static bool 
is_open(game_t* game, const int c, const int r){

    if (game == NULL) {
        fprintf(stderr, "game pointer was null\n");
	    exit(1);
    }    
    if( '.' == get_grid_value(game, r, c) || '*' == get_grid_value(game, r, c)){
        return true;
    }
    else if( isalpha(get_grid_value(game,r,c))){
        return true;
    }
    else if ('+' == get_grid_value(game, r, c) || '|' == get_grid_value(game, r, c) || '-' == get_grid_value(game, r, c) || ' ' == get_grid_value(game, r, c)){
        return false;
    }
    return false;
}

/**************** is_integer ****************/
static bool is_integer(float num)
{
    int convertedNum = (int)num;
    return (convertedNum == num);
}

/**************** is_gridspot ****************/
static bool 
is_gridspot(double a, double b)
{
    return (is_integer(a) && is_integer(b));
}

/**************** is_visible ****************/
bool 
is_visible(game_t* game, const int playerColumn, const int playerRow, const int column, const int row) 
{

    if (game == NULL) {
        fprintf(stderr, "game pointer was null\n");
	    exit(1);
    }      

    // calculate difference between where the player is and where the spot is
    double changeY = row - playerRow;
    double changeX = column - playerColumn;
    double slope = 0;
    double constant, columnStart, columnEnd, rowStart, rowEnd;

    // rowStart and rowEnd determine what points we look between for vertical visibility, columnStart and columnEnd determin whatt points we look between for horizontal visibility

    // if the x coordinate doesn't change (the line is vertical)
    if (changeX == 0) { 
        columnStart = 0;
        columnEnd = 0;

        // either move up or down based on where the player is 
        if (playerRow > row) {
            rowStart = row + 1;
            rowEnd = playerRow;
        } else {
            rowStart = playerRow + 1;
            rowEnd = row;
        }
    // if the y coordinate doesn't change (the line is horizontal)
    } else if (changeY == 0) { 

        rowStart = 0;
        rowEnd = 0;

        // either move left or right based on where the player is at 
        if (playerColumn > column) {
            columnStart = column + 1;
            columnEnd = playerColumn;

        } else {
            columnStart = playerColumn + 1;
            columnEnd = column;
        }
    
    // if both the x and y coordinate change (the line is diagonal)
    } else {
        // calculate slope: rise over run
        slope = (double) (changeY / changeX);
        constant = playerRow - (slope * playerColumn);
        columnStart = 0;
        columnEnd = 0;
        rowStart = 0;
        rowEnd = 0;

        // if the player is to the right of the spot, start from one to the right of the current column, loop to one to the left of the player column
        if (playerColumn > column) {
            columnStart = column + 1;
            columnEnd = playerColumn;

         // if the player is to the left of the spot, start from one to the right of the player column, loop to one to the left of the spot column
        } else if (playerColumn < column) {
            columnStart = playerColumn + 1;
            columnEnd = column;
        }

        // if the player is above the spot, start from one above the spot and loop to one below the player row
        if (playerRow > row) {
            rowStart = row + 1;
            rowEnd = playerRow;

         // if the player is above the spot, start from one above the spot and loop to one below the player row
        } else if (playerRow < row) {
            rowStart = playerRow + 1;
            rowEnd = row;
        }
    }

    // for every point from the column start to end
    for (; columnStart < columnEnd; columnStart++) {
 
        double newY;

        // if the line is horizontal
        if (rowStart == 0 && rowEnd == 0) {
            // the "intersecting" y value is the same as the player row
            newY = playerRow;
        } else {
            // otherwise, calculate the intersecting y value
            newY = slope * columnStart + constant;
        }
        // check if what's located at this y value is 
        if (is_gridspot(columnStart, newY)) {
            if (!is_open(game, columnStart, newY)) {
                return false;
            }
        } else {
            // if not a grid point, check the top and bottom
            if (!(is_open(game, columnStart, floor(newY)) || is_open(game, columnStart, ceil(newY)))) {
                return false;
            }
        }
    }

    for (; rowStart < rowEnd; rowStart++) {
        double newX;

        // if the line is vertical
        if (columnStart == 0 && columnEnd == 0) {
            newX = playerColumn;
        } else if (slope != 0) {
            newX = (rowStart - constant) / slope;
        }

        // check that both points are ints
       
        if ((is_gridspot(newX, rowStart))){
            // if it's not an "open" point, then return false
            if (!is_open(game, newX, rowStart)) {
                return false;
            }
        }
        else {
            // if both points are not ints, check the top and bottom for obstructions-- if both are not open, then there is an obstruction
            if (!(is_open(game, floor(newX), rowStart) || is_open(game, ceil(newX), rowStart))) {
                return false;
            }
        }
    }
    // spot is visible
    return true; 
}
/**************** get_player_visible ****************/
bool 
get_player_visible(game_t* game, client_t* player)
{
    bool modified = false;
    int pr = player->r;
    int pc = player->c;

    for (int r = 0; r < game->rows; r++){
        for (int c = 0; c < game->columns; c++){
            // don't compute anything if the game grid space is empty
            if (isspace(game->grid[r][c])){
                continue;
            }
            if (r == pr && c == pc){
                if (player->grid[r][c] != '@'){
                    modified = true;
                }

                player->grid[r][c] = '@';
            }
            else if (is_visible(game, pc, pr, c, r)){
                if (player->grid[r][c] != game->grid[r][c]){
                    modified = true;
                }
                player->grid[r][c] = game->grid[r][c];
            }
            else if (player->grid[r][c] == '*'){
                player->grid[r][c] = '.';
                modified = true;
            }
            else if (isalpha(player->grid[r][c])){
                // get the player
                client_t* other_player = find_player(player->grid[r][c], game);
               
                // check if tunnel and change spot accordingly
                if (other_player->onTunnel){
                    player->grid[r][c] = '#';
                }
                else{
                    player->grid[r][c] = '.';
                }
                modified = true;
            }

            
        }
    }

    return modified;                                                                                                                                                                             
}

/**************** grid_delete ****************/
void
grid_delete(char** grid, int rows)
{
    for (int r = 0; r < rows; r++){
        mem_free(grid[r]);
    }

    mem_free(grid);
}