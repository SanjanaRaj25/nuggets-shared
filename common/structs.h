/*
* structs.h
* File containing data structures necessary for game operation
*
* Team 9: Plankton, May 2023
*/


#ifndef __STRUCTS_H
#define __STRUCTS_H

#include <stdlib.h>
#include <stdbool.h>
#include "../support/message.h"


// Containing information about nugget piles
typedef struct gold_location {
    int r; // the row its in
    int c; // the column its in
    int nuggetCount; // how many nuggets are there
} gold_location_t;

// Holds client specific info
typedef struct client {
    addr_t clientAddr; // the address to reach the client
    bool isSpectator; // is the client a spectator
    char id;  // player ID letter, only relevant for players not spectators
    char* real_name; // player's name
    int gold;  // how much gold they have collected
    int r;  // the row they are in
    int c;  // the column they are in
    char** grid;  // the player specific grid map
    bool onTunnel;  // is the player standing in a tunnel
    int clientsArr_Idx;  // the index of the player in the game structs clients array
    bool quit;  // has this client quit the game
    
} client_t;

// Holds game relevant information
typedef struct game {
    char** grid;  // the global grid map
    client_t** clients; // the array of clients that have joined
    int goldRemaining;  // how much gold is left
    int playersJoined;  // how many players have joined
    bool spectatorActive;  // is there a spectator currently active
    int rows;  // how many rows does the grid have
    int columns;  // how many columns does the grid have
    gold_location_t** locations;  // array of gold nugget location structs
    int totalGoldPiles;  // how many piles of nuggets there are

} game_t;

#endif