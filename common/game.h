/* 
* game.h - header for module handling game and client methods
* includes functions to modify the overall status of the game, as well as locate entities within the game
*
* Team 9: Plankton, May 2023
*/

#ifndef __GAME_H_
#define __GAME_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../libs/file.h"
#include "../libs/mem.h"

#include "../support/log.h"
#include "../support/message.h"

#include "structs.h"
#include "grid.h"


/**************** Functions ****************/


/* new_player
 * Creates a new player client and adds it to the game.
 * Inputs:
 *     - game: pointer to the game object
 *     - client: address of the client
 *     - name: name of the player
 * Outputs:
 *     - Returns the newly created player client.
 * Notes: The returned client_t* must be freed with delete_client
 */
client_t* new_player(game_t* game, addr_t client, char* name);

/* update_position
 * Updates the position of the player client by passing new row and column coordinates into the player structure.
 * Inputs:
 *     - player: pointer to the player client
 *     - r: row coordinate
 *     - c: column coordinate
 * Outputs:
 *     - None.
 */
void update_position(client_t* player, int r, int c);

/* find_client
 * Finds a client in the game given an address.
 * Inputs:
 *     - clientAddr: address of the client
 *     - game: pointer to the game object
 * Outputs:
 *     - Returns the client if found, NULL otherwise.
 * Notes useful for finding the client associated with a received message.
 */
client_t* find_client(addr_t clientAddr, game_t* game);

/* find_player
 * Finds a player client in the game given a player ID char.
 * Inputs:
 *     - id: player ID
 *     - game: pointer to the game object
 * Outputs:
 *     - Returns the player client if found, NULL otherwise.
 */
client_t* find_player(char id, game_t* game);

/* new_spectator
 * Creates a new spectator client and adds it to the game. 
 * Inputs:
 *     - game: pointer to the game object
 *     - client: address of the client
 * Outputs:
 *     - Returns the newly created spectator client.
* Notes: The returned client_t* must be freed with delete_client. Existing spectator is not removed by the function and must be done prior by the caller.
 */
client_t* new_spectator(game_t* game, const addr_t client);

/* delete_client
 * Deletes a client from the game, freeing associated memory.
 * Inputs:
 *     - client: pointer to the client to be deleted
 *     - game: pointer to the game object
 * Outputs:
 *     - None.
 */
void delete_client(client_t* client, game_t* game);

/* new_game
 * Creates a new game struct. 
 * Loads the map file, initializes game statistics to 0, and initializes a NULL list of clients.
 * Inputs:
 *     - map_file: pointer to the file containing the game map
 *     - maxPlayers: maximum number of players allowed in the game
 * Outputs:
 *     - Returns the newly created game object.
 * Notes: the game_t* must be freed at some point by the caller using end_game.
 */
game_t* new_game(FILE* map_file, const int maxPlayers);

/* end_game
 * Ends the game and deallocates the global grid and each of the clients.
 * Inputs:
 *     - game: pointer to the game object
 *     - maxGoldPiles: maximum number of gold piles in the game
 * Outputs:
 *     - None.
 */
void end_game(game_t* game, int maxGoldPiles);

/* update_gold
 * Updates amount of gold in a player's purse, the overall amount of gold remaining in the game, and grid representations.
 * Inputs:
 *     - game: pointer to the game object
 *     - player: pointer to the player client
 *     - r: row coordinate
 *     - c: column coordinate
 *     - goldMaxPiles: maximum number of gold piles in the game
 * Outputs:
 *     - Returns the amount of gold obtained by the player.
 * Notes: this function will return -1 if the location passed in was not a gold location.
 */
int update_gold(game_t* game, client_t* player, int r_pos, int c_pos, int goldMaxPiles);

/* load_gold
 * Loads the initial gold configuration in the game. 
 * Specifically, places gold into random bundles at random locations at the beginning of the game, by calling `add_gold_pile`.
 * Inputs:
 *     - game: pointer to the game object
 *     - goldTotal: total amount of gold in the game
 *     - goldMinPiles: minimum number of gold piles in the game
 *     - goldMaxPiles: maximum number of gold piles in the game
 * Outputs:
 *     - None.
 */
void load_gold(game_t* game, const int goldTotal, const int goldMinPiles, const int goldMaxPiles);

/* add_gold_pile
 * Adds a new gold pile to the game at a random location.
 * Inputs:
 *     - game: pointer to the game object
 *     - gold_amt: amount of gold in the new pile
 *     - piles: number of gold piles
 * Outputs:
 *     - None.
 */
void add_gold_pile(game_t* game, int gold_amt, int piles);

/* nugget_count_array
 * Creates an array containing the randomly generated amounts of gold nuggets in each pile.
 * Inputs:
 *     - goldMinPiles: minimum number of gold piles in the game
 *     - goldMaxPiles: maximum number of gold piles in the game
 *     - goldTotal: total amount of gold in the game
 * Outputs:
 *     - Returns an array containing the number of gold nuggets in each pile.
 * Notes: Caller is responsible for freeing the array.
 */
int* nugget_count_array(const int goldMinPiles, const int goldMaxPiles, int goldTotal);

#endif // __GAME_H_