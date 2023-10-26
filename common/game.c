/*
 * game.c 
 * module handling game and client methods
 * includes functions to modify the overall status of the game, as well as locate entities within the game
 * specific function descriptions are located in game.h
 * Team 9: Plankton, May 2023
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../libs/file.h"
#include "../libs/mem.h"

#include "structs.h"
#include "game.h"
#include "grid.h"

/**************** new_player ****************/
client_t*
new_player(game_t* game, addr_t client, char* name)
{
    // allocate memory for a new client, of type player
    client_t* player = mem_malloc_assert(sizeof(client_t), "Error allocating memory in new_player.\n");
    char* alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    // store their address, whether they are a spectator, and their ID based on the time of them joining
    player->clientAddr = client;
    player->isSpectator = false;
    player->id = alpha[game->playersJoined];
    // assign their name
    player->real_name = mem_malloc_assert(strlen(name) + 1, "Error allocating memory in new_player.\n");
    strcpy(player->real_name, name);
    // initially give them 0 gold, and load in an empty grid, mark that they are not on a tunnel (always true at start)
    player->gold = 0;
    player->grid = load_player_grid(game);
    player->onTunnel = false;
    // update the number of clients in the game
    game->clients[game->playersJoined + 1] = player;
    // have their player store their index in the array of clients
    player->clientsArr_Idx = game->playersJoined + 1;
    (game->playersJoined)++;

    player->quit = false;
    
    // assign player to a random spot, then update their grid to reflect what is visible to them
    assign_random_spot(game->grid, game->rows, game->columns, player->id, &player->r, &player->c);
    get_player_visible(game, player);

    return player;
}

/**************** update_position ****************/
void
update_position(client_t* player, int r, int c)
{
    // set the row and column stored within the client struct to the row and column that is passed in
    player->r = r;
    player->c = c;
}

/**************** find_client ****************/
client_t*
find_client(addr_t clientAddr, game_t* game)
{
    // loop over all the clients in the game and return the one with the address that was passed in
    for (int i = 0; i < game->playersJoined + 1; i++){
        if ((game->clients)[i] != NULL){
            if (message_eqAddr(((game->clients)[i])->clientAddr, clientAddr)){
                return (game->clients)[i];
                break;
            }   
        } 
    }
    return NULL;
}

/**************** find_ player ****************/
client_t*
find_player(char id, game_t* game)
{
    // find the player with the passed in ID
    for (int i = 1; i < game->playersJoined + 1; i++){
        if ((game->clients)[i] != NULL){
            if (((game->clients)[i])->id == id){
                return (game->clients)[i];
                break;
            }
        }
    }
    return NULL;
}

/**************** new_spectator ****************/
client_t*
new_spectator(game_t* game, const addr_t client)
{
    // allocate memory for a new client, of type spectator
    client_t* spectator = mem_malloc_assert(sizeof(client_t), "Error allocating memory in new_spectator.\n");
    // mark that this client is a spectator, store the appropriate address and id
    spectator->isSpectator = true;
    spectator->clientAddr = client;
    spectator->id = '$';
    // they should not have a local grid, gold, or name
    spectator->grid = NULL;
    spectator->gold = 0;
    spectator->real_name = NULL;
    spectator->onTunnel = false;
    // the spectator is located in the 0th index of the client array
    spectator->clientsArr_Idx = 0;
    (game->clients)[0] = spectator;
    game->spectatorActive = true;
    spectator->quit = false;
    // return spectator 
    return spectator;
}

/**************** delete_client ****************/
void
delete_client(client_t* client, game_t* game)
{
    // if the name stored within the client is not null, free it
    if (client->real_name != NULL){
        mem_free(client->real_name);
    }

    // if the grid stored within the client is not null, free it
    if (client->grid != NULL){
        grid_delete(client->grid, game->rows);
    }

    // set the client in the clients array to null
    (game->clients)[client->clientsArr_Idx] = NULL;

    // free the client object
    mem_free(client);
}

/**************** new_game ****************/
game_t*
new_game(FILE* map_file, const int maxPlayers)
{
    // allocate memory for a new game object and a new clients array
    game_t* new_game = mem_malloc_assert(sizeof(game_t), "Error allocating memory in new_game.\n");
    new_game->clients =  mem_malloc_assert((maxPlayers + 1) * sizeof(client_t*), "Error allocating memory in new_game.\n");

    // initialize array of clients to be all NULL
    for (int i = 0; i < maxPlayers + 1; i++){
        (new_game->clients)[i] = NULL;
    }

    // set all game variables to 0
    new_game->goldRemaining = 0;
    new_game->playersJoined = 0;
    new_game->spectatorActive = false;

    // load in the map
    new_game->grid = load_grid(map_file, &(new_game->rows), &(new_game->columns));

    // return the game object
    return new_game;
}

/**************** end_game ****************/
void
end_game(game_t* game, int maxGoldPiles)
{

    // loop over all the clients in the game and delete them
    for (int i = 0; i < game->playersJoined + 1; i++){
        client_t* client = game->clients[i];
        if (client != NULL){
            delete_client(client, game);
        }
    }

    // free the clients array
    mem_free(game->clients);

    // if the grid within the game struct is not null, delete it
    if (game->grid != NULL){
        grid_delete(game->grid, game->rows);
    }

    // if the gold locations array within the game is not null, free everything stored within it
    if (game->locations != NULL){
        for (int i = 0; i < game->totalGoldPiles; i++){
            mem_free(game->locations[i]);
        }

        // free the locations array itself 
        mem_free(game->locations);
    }
    
    // free the game struct
    mem_free(game);
}

/**************** update_gold ****************/
int
update_gold(game_t* game, client_t* player, int r, int c, int goldMaxPiles)
{
    // loop over all the gold piles
    for (int i = 0; i < goldMaxPiles; i++){
        gold_location_t* location = game->locations[i];

        if (location->nuggetCount < 0){
            break; // there is no such gold location if this condition is reached
        }

        // if the pile matches the row and column that was passed in
        if (location->r == r && location->c == c){
            // subtract the amount of gold in that pile from the overall gold in the game
            game->goldRemaining -= location->nuggetCount;
            // add the amount of gold in that pile to the player's purse
            player->gold += location->nuggetCount;            
            // return the leftover amount of gold in that location
            return location->nuggetCount;
        }
    }
    return -1; // error meaning no such gold location exists
}

/**************** load_gold ****************/
void
load_gold(game_t* game, const int goldTotal, const int goldMinPiles, const int goldMaxPiles)
{
    int gold_amt;
    
    // allocate memory for an array within the game struct that holds gold locations
    game->locations =  mem_malloc_assert((goldMaxPiles) * sizeof(gold_location_t*), "Error allocating memory in load_gold.\n");

    int* nugget_counts = nugget_count_array(goldMinPiles, goldMaxPiles, goldTotal);

    // loop over all the possible gold piles
    for (int i = 0; i < goldMaxPiles; i++){
        // assign them gold according to what's stored at the nugget count array, in the same index
        gold_amt = nugget_counts[i];

        // if the amount is less than or equal to zero, then there is no gold leftover, and we break the loop because we don't need to assign any more
        if (gold_amt <= 0){
            game->totalGoldPiles = i;
            break;
        }

        // add this gold pile, with this amount and index, to the game
        add_gold_pile(game, gold_amt, i);
    }

    // free the nugget_counts array because it isn't needed anymore
    mem_free(nugget_counts);   
}

/**************** add_gold_pile ****************/
void 
add_gold_pile(game_t* game, int gold_amt, int piles)
{

    // allocate memory for a gold_location type object that stores where a gold pile is located
    gold_location_t* gold_spot = mem_malloc_assert(sizeof(gold_location_t), "Error allocating memory in add_gold_pile.\n");
    // assign it to a random open spot in the grid
    assign_random_spot(game->grid, game->rows, game->columns, '*', &(gold_spot->r), &(gold_spot->c));
    // assign it a gold amount, then update the game variable goldRemaining accordingly
    gold_spot->nuggetCount = gold_amt;
    game->goldRemaining += gold_amt;
    
    (game->locations)[piles] = gold_spot;
}

/**************** nugget_count_array ****************/
int*
nugget_count_array(const int goldMinPiles, const int goldMaxPiles, int goldTotal)
{
    const int lower_bound = 5;
    const int upper_bound = 30;

    while (true){
        int piles = 0;
        int* arr = mem_malloc_assert(goldMaxPiles * sizeof(int), "Error allocating memory in nugget_count_array.\n");
        int gold_amt;
        int total_gold_added = 0;
        
        // create gold piles and add gold to them, until you reach the maximum number of piles or the maximum amount of gold
        while (piles < goldMaxPiles && total_gold_added < goldTotal){
            // generate a pseudo-random number between the upper and lower bound that represents the amount of gold that will be assigned to this pile
            gold_amt = (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
            // if this + the gold already added would be greater than the allowed total, or we have reached the max # of gold piles
            if (gold_amt + total_gold_added > goldTotal || piles == goldMaxPiles - 1){
                // make the amount the max that can be added
                gold_amt = goldTotal - total_gold_added;
            }
            // add the gold amount to the array, and to the total amount of gold added
            arr[piles] = gold_amt;
            total_gold_added += gold_amt;
            piles++;
        }
        // if this is greater or equal to the minimum # of gold piles
        if(piles >= goldMinPiles){
            // and less than the max number of piles -1
            if (piles < goldMaxPiles - 1){
                // set this index of the array to -1
                arr[piles] = -1;
            }
            return arr;
        }
        mem_free(arr);
    }
}

