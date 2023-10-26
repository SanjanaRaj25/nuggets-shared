/*
server.c 
server module for nuggets game: handles overall game logic and all communication with client

Team 9: Plankton, May 2023
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>



#include "../libs/file.h"
#include "../libs/mem.h"
#include "../support/log.h"
#include "../support/message.h"
#include "../common/game.h"
#include "../common/grid.h"


/**************** game variables  ****************/
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles


/**************** function prototypes  ****************/
bool handleMessage(void* arg, const addr_t from, const char* message);
void update_displays(game_t* game, int r1, int c1, int r2, int c2);
void inform_newClient(client_t* client, game_t* game);
void send_goldMsg(game_t* game, client_t* client, int goldPickedUp);
void send_displayMsg(game_t* game, client_t* client);
char* extract_playerName(const char* message, addr_t clientAddr);
char* extractRequest(const char* input);
int handle_movement(client_t* player, char key, game_t* game);
static void update_previous_spot(client_t* player, game_t* game, char grid_val);
void send_quitMsg(addr_t clientAddr, int quitCode, bool isSpectator);
void quit_all(game_t* game, int maxPlayers);
void handle_quit(client_t* player, game_t* game);
void send_gameOverMsg(game_t* game, int maxNameLength);


/**************** functions ****************/
/**
 * @brief Parses arguments, and starts up the game.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int
main(const int argc, char* argv[])
{

    // parse args: first argument should be the pathname for a map file, the second is an optional seed for the random-number generator, which must be a positive int if provided
    
    // make sure there are no more than 2 arguments
    if (argc != 3 && argc != 2){
        fprintf(stderr, "Invalid number of arguments provided. Call using the format ./server map.txt [seed]\n");
	    exit(1);
    }

    // parse the command line, open the file
    char* mapFilename = argv[1];
    FILE* map_file;
    map_file = fopen(mapFilename, "r");

    // defensive check: file could be opened
    if (map_file == NULL){
        fprintf(stderr, "Error. File could not be opened\n");
	    exit(1);
    }


    // if the user provided a seed and it's a valid number, use it to initialize the random sequence:
    if (argc == 3 && (atoi(argv[2]) != 0)) {
        srand(atoi(argv[2]));
    }

    // if they did not, seed the random-number generator with the process id
    else {
        srand(getpid());
    }

    // create a new game first
    game_t* game = new_game(map_file, MaxPlayers);
    load_gold(game, GoldTotal, GoldMinNumPiles, GoldMaxNumPiles);

    // start logging
    FILE* fp = fopen("server.log", "w");
    flog_init(fp);

    // start up message module
    message_init(stderr);
    message_loop(game, 0, NULL, NULL, handleMessage);
    message_done();
    end_game(game, GoldMaxNumPiles);

    // close the file
    fclose(map_file);

    // stop logging
    flog_done(fp);
    fclose(fp);
    return(0);
}

/**
 * @brief Handles messages to `PLAY`, `SPECTATE`, or a specific `KEY` sent by a client. 
 * The function allows new players to join the game if the maximum amount of player has not been reached,
 * ensures the most recent spectator is observing the game, and calls `handle_movement` for any key press.
 * 
 * @param arg - the game_t struct holding game information
 * @param from - the address of the client who sent the message
 * @param message - the message string sent from the client
 * @return true 
 * @return false 
 */
bool
handleMessage(void* arg, const addr_t from, const char* message)
{
    game_t* game = arg;
    char* request = extractRequest(message);

    if (strcmp(request, "PLAY") == 0){
        mem_free(request);
        if (game->playersJoined < MaxPlayers){

            char* name = extract_playerName(message, from); 
            if (name == NULL){
                FILE* fp = fopen("server.log", "w");
                flog_v(fp, "player name was invalid");
                fclose(fp);
                return false;
            }

            client_t* player = new_player(game, from, name);
            mem_free(name);

            char* response = mem_malloc_assert(5, "Error allocating memory in handleMessage.\n");
            sprintf(response, "OK %c", player->id);

            message_send(from, response);
            mem_free(response); 

            // send new client messages: grid, gold, display
            inform_newClient(player, game);

            update_displays(game, player->r, player->c, -1, -1);

        }
        else {
            send_quitMsg(from, 2, false);
        }
    }
    else if (strcmp(request, "SPECTATE") == 0){
        mem_free(request);
        if (game->spectatorActive && game->clients[0] != NULL){
            send_quitMsg(game->clients[0]->clientAddr, 0, true);
            delete_client((game->clients)[0], game);
            game->spectatorActive = false;
        }

        client_t* spectator = new_spectator(game, from);

        // send new client messages: grid, gold, display
        inform_newClient(spectator, game);

    }
    else if (strcmp(request, "KEY") == 0){
        mem_free(request);

        if (strlen(message) != 5 || !isalpha(message[4])){
            FILE* fp = fopen("server.log", "w");
            flog_e(fp, "message was malformed");
            fclose(fp);
        }

        client_t* player = find_client(from, game);

        // returns true when all gold is found
        if (tolower(message[4]) == 'q'){
            handle_quit(player, game);
        }
        else{
            int movementCode = handle_movement(player, message[4], game);
            // signifies all gold was found
            if (movementCode == 2){
                return true; // causes message loop to end
            }
            // if we were able to move and the the key was uppercase
            else if (movementCode != 1 && isupper(message[4])){
                // keep moving while possible
                while (movementCode == 0){
                    movementCode = handle_movement(player, message[4], game);
                }
            }
            if (movementCode == 2){ // handles edge case of shift moving into the last gold
                return true; // causes message loop to end
            }
        }
    }
    else{
        // log error and move on
        FILE* fp = fopen("server.log", "w");
        flog_e(fp, "message was malformed");
        fclose(fp);
    }
    return false;
}

/**
 * @brief Sends a message to all players and the spectator to update their local displays by calling `send_displayMsg`.
 * 
 * @param game - the game_t struct holding game information
 * @param takes in two positions to check if they changed for the player, second point is optional and will be ignored if -1 is passed for r2 and c2
 */
void
update_displays(game_t* game, int r1, int c1, int r2, int c2)
{
    // update spectator if there is one no matter what
    if (game->spectatorActive){
        send_displayMsg(game, game->clients[0]);
    }

    // used to check whether or not the points that changed are visible for the player
    bool point1_vis = false;
    bool point2_vis = false; // this second point is options

    for (int i = 1; i < game->playersJoined + 1; i++){
        client_t* player = game->clients[i];

        if (player != NULL && !player->quit){
            point1_vis = !isspace(player->grid[r1][c1]); // checks if the player can see that point
            if (r2 != -1){ //point2_vis is set to true if -1 is passed otherwise we check if it is visible
                point2_vis = !isspace(player->grid[r2][c2]);
            }
            else {
                point2_vis = true;
            }
            // only update visibility if points changed are currently in sight
            if (point1_vis && point2_vis){
                if (get_player_visible(game, player)){
                    send_displayMsg(game, player); // only send a new message if their display changes
                }
            }

        }
    }
}

/**
 * @brief - Sends new clients a gold message by calling `send_goldMsg` and a display message by calling `send_displayMsg`.
 * 
 * @param client - the client_t struct holding information about the new client
 * @param game -the game_t struct holding game information
 */
void
inform_newClient(client_t* client, game_t* game)
{
    // send grid message
    char* gridMsg = mem_malloc_assert(13, "Error allocating memory in inform_newClient.\n"); // allows for max 3 digit row, columns numbers
    // check size of rows, columns here
    
    sprintf(gridMsg, "GRID %d %d", game->rows, game->columns);
    message_send(client->clientAddr, gridMsg);
    mem_free(gridMsg);

    // send gold message
    send_goldMsg(game, client, 0);

    // send display message
    send_displayMsg(game, client);

}

/**
 * @brief Informs players of the amount of gold they pick up, the amount in their purses, and both players and spectators of the amount left in the game.
 * 
 * @param game - the game_t struct holding game information
 * @param client - the client_t struct holding information about the new client
 * @param goldPickedUp - the amount of gold the client just picked up
 */
void
send_goldMsg(game_t* game, client_t* client, int goldPickedUp)
{
    // send gold message
    char* goldMsg = mem_malloc_assert(17, "Error allocating memory in inform_newClient.\n"); // allows for max 3 digit gold count numbers
    sprintf(goldMsg, "GOLD %d %d %d", goldPickedUp, client->gold, game->goldRemaining); //initial gold message always 0 just picked up
    message_send(client->clientAddr, goldMsg);
    mem_free(goldMsg);

}

/**
 * @brief Sends message to update a client's local display.
 * 
 * @param game - the game_t struct holding game information
 * @param client - the client_t struct holding information about the new client
 */
void
send_displayMsg(game_t* game, client_t* client)
{
    int msgSize;
    char* map;
    char* display;

    if (client->isSpectator){
        map = grid_toStr(game->grid, NULL, game->rows, game->columns);
    }
    else{ // assmumes that grid is already up to date
        map = grid_toStr(game->grid, client->grid, game->rows, game->columns);
    }

    msgSize = 10 + strlen(map);
    display = mem_malloc_assert(msgSize, "Error allocating memory in sendDisplayMsg.\n");
    sprintf(display, "DISPLAY\n%s", map);
    mem_free(map);

    message_send(client->clientAddr, display);
    mem_free(display);

}

/**
 * @brief Extracts each player's name when they first join the game, storing the string up to a maximum of 50 characters.
 * 
 * @param message - the message string received from a client
 * @param clientAddr - the client's address
 * @return char* - the name extracted from the message
 */
char*
extract_playerName(const char* message, addr_t clientAddr)
{
    bool reachedNameStart = false;
    bool emptyName = true;
    char* name = mem_malloc_assert(MaxNameLength + 1, "Error allocating memory in extract_playerName.\n");
    int curr_nameLength = 0;

    for (int i = 0; i < (int)strlen(message); i++){
        if (isspace(message[i]) && !reachedNameStart){
            reachedNameStart = true;
            continue;
        }
        else if (reachedNameStart && !isspace(message[i]) && emptyName){
            emptyName = false;
        }

        if (reachedNameStart){
            if (curr_nameLength < MaxNameLength){
                if(!isgraph(message[i]) && !isblank(message[i])){
                    name[curr_nameLength] = '_';
                }
                else{
                    name[curr_nameLength] = message[i];
                }
                curr_nameLength++;
            }
            else{
                break;
            }
        }
    }
    name[curr_nameLength] = '\0';
    
    if(emptyName){
        send_quitMsg(clientAddr, 3, false);
        mem_free(name);
        return NULL;
    }

    return name;
}

/**
 * @brief Called by `handleMessage` to handle each request (message) of a client playing the game.
 * 
 * @param input - the whole string received from the client
 * @return char* - the extracted request string sent from the client
 */
char* 
extractRequest(const char* input) 
{
    if (input == NULL || input[0] == '\0') {
        // Handle empty input string or null pointer
        return NULL;
    }

    int length = strlen(input);
    int wordEndIndex = -1;

    // Find the end index of the first word
    for (int i = 0; i < length; i++) {
        if (input[i] == ' ') {
            wordEndIndex = i;
            break;
        }
    }

    if (wordEndIndex == -1) {
        // If no space is found, the entire input is considered as the first word
        wordEndIndex = length;
    }

    // Allocate memory for the new string
    char* firstWord = mem_malloc_assert((wordEndIndex + 1) * sizeof(char), "Error allocating memory in extractFirstWord.\n");


    // Copy the first word into the new string
    strncpy(firstWord, input, wordEndIndex);
    firstWord[wordEndIndex] = '\0';  // Null-terminate the string

    return firstWord;
}

/**
 * @brief Sends quitting message and removes a client from the game, updating the displays of remaining clients to reflect changes.
 * 
 * @param game - the game_t struct holding game information
 * @param player - the client_t struct holding information about the new client
 */
void 
handle_quit(client_t* player, game_t* game)
{
    send_quitMsg(player->clientAddr, 1, player->isSpectator);
    
    if (!player->isSpectator){
        // reset spot
        if (player->onTunnel){
            change_spot(game, player->r, player->c, '#');
        }
        else{
            change_spot(game, player->r, player->c, '.');
        }
    }

    player->quit = true;

    update_displays(game, player->r, player->c, -1, -1);
}

/**
 * @brief Handles the action a player whenever they press `Q` for quitting or a key for moving within the grid. 
 * When moving, the function updates a player's position, visibility, and handles the cases when a player steps on gold or another player.
 * 
 * @param player- the client_t struct holding information about the new client
 * @param key - the key entered by the client
 * @param game - the game_t struct holding game information
 * @return int - a code representing the result of the move: 0 means success, 1 means unable to move, 2 means game over
 */
int
handle_movement(client_t* player, char key, game_t* game)
{

    int newPos_r = player->r;
    int newPos_c = player->c;

    switch (tolower(key)) {
        // update new Pos based on the key inputted
        case 'h': newPos_c--; break;
        case 'l': newPos_c++; break;
        case 'j': newPos_r++; break;
        case 'k': newPos_r--; break;
        case 'y': newPos_c--; newPos_r--; break;
        case 'u': newPos_c++; newPos_r--; break;
        case 'b': newPos_c--; newPos_r++; break;
        case 'n': newPos_c++; newPos_r++; break;
        default: return 1;
    }

    char grid_val = get_grid_value(game, newPos_r, newPos_c);

    if (grid_val == '+' || grid_val == '-' || grid_val == '|' || grid_val == ' '){
        return 1; // code meaning unable to move
    }
    else if (grid_val == '.' || grid_val == '#'){
        // change the spot the player came from back
        update_previous_spot(player, game, grid_val);
        
        // change the global grid on the spot they are now on to be their letter
        change_spot(game, newPos_r, newPos_c, player->id);

        int pr = player->r;
        int pc = player->c;

        // update the player's position in the player struct
        update_position(player, newPos_r, newPos_c);

        // call update function
        update_displays(game, pr, pc, newPos_r, newPos_c);
        
    }
    else if (isalpha(grid_val)){
        // find the player there using a game function
        client_t* other_player = find_player(grid_val, game);

        // switch the positions of the two players
        update_position(other_player, player->r, player->c);
        update_position(player, newPos_r, newPos_c);

        // update the player's records on the spot they stand on 
        bool player_OnTunnel = player->onTunnel;
        player->onTunnel = other_player->onTunnel;
        other_player->onTunnel = player_OnTunnel;

        // update the global grid to reflect change
        change_spot(game, player->r, player->c, player->id);
        change_spot(game, other_player->r, other_player->c, other_player->id);

        // call update function
        update_displays(game, other_player->r, other_player->c, player->r, player->c);


    }
    else if (grid_val == '*'){
        int nuggetsFound = update_gold(game, player, newPos_r, newPos_c, GoldMaxNumPiles);

        if (nuggetsFound < 0){
            // attempted to access a spot that wasn't a gold location
            FILE* fp = fopen("server.log", "w");
            flog_e(fp, "tried to access a spot that wasn't a gold location.");
            fclose(fp);
            return 0;
        }
        
        // update the client that just picked up gold
        send_goldMsg(game, player, nuggetsFound);

        // update the other clients about the gold counts
        for (int i = 0; i < game->playersJoined + 1; i++){
            if (game->clients[i] != NULL && ((game->clients)[i])->id != player->id && !((game->clients[i])->quit)){
                send_goldMsg(game, (game->clients)[i], 0);
            }
        }
                
        // change the spot the player came from back
        update_previous_spot(player, game, grid_val);
        
        // change the global grid on the spot they are now on to be their letter
        change_spot(game, newPos_r, newPos_c, player->id);

        int pr = player->r;
        int pc = player->c;

        // update the player's position in the player struct
        update_position(player, newPos_r, newPos_c);

        // call update function
        update_displays(game, pr, pc, newPos_r, newPos_c);


        if (game->goldRemaining == 0){
            send_gameOverMsg(game, MaxNameLength);
            return 2; // code meaning game over
        }

    }
    

    return 0; // code meaning sucessful move 
}

/**
 * @brief Sends quit messages to all the clients when the game is over.
 * 
 * @param game - the game_t struct holding game information
 * @param maxPlayers - the maximum players that can join
 */
void quit_all(game_t* game, int maxPlayers)
{
    for (int i = 0; i < maxPlayers + 1; i++){
        client_t* client = game->clients[i];
        if (client != NULL && !client->quit){
            send_quitMsg(client->clientAddr, 1, client->isSpectator);
        }
    }

}


/**
 * @brief Updates the previous spot occupies by a player when they move to the next spot.
 * 
 * @param player- the client_t struct holding information about the new client
 * @param game - the game_t struct holding game information
 * @param grid_val - the char representing the current spot the player is on
 */
static void
update_previous_spot(client_t* player, game_t* game, char grid_val)
{
    //change the global grid on the spot they came from back to what it was
    if (player->onTunnel){
        change_spot(game, player->r, player->c, '#');
    }
    else{
        change_spot(game, player->r, player->c, '.');
    }
    player->onTunnel = (grid_val == '#');
}

/**
 * @brief Sends message to client about to quit the game, according to their role (player or spectator) and the reason for quitting.
 * 
 * @param clientAddr - the address the reach the client
 * @param quitCode - a code representing why the client quit: 0 means spectator was replaced, 1 mean player quit, 2 means full game, 3 means invalid player name
 * @param isSpectator - boolean, is the client a spectator
 */
void
send_quitMsg(addr_t clientAddr, int quitCode, bool isSpectator)
{
    char* quitMsg = mem_malloc_assert(55, "Error allocating memory in send_quitMsg.\n"); 
    char* quitReason = mem_malloc_assert(45, "Error allocating memory in send_quitMsg.\n");

    if (isSpectator){
        if (quitCode == 0){
            strcpy(quitReason, "You have been replaced by a new spectator.");
        }
        else{
            strcpy(quitReason, "Thanks for watching!");
        }
    }
    else {
        if (quitCode == 1){
            strcpy(quitReason, "Thanks for playing!");
        }
        if (quitCode == 2){
            strcpy(quitReason, "Game is full: no more players can join.");
        }
        if (quitCode == 3){
            strcpy(quitReason, "Sorry - you must provide player's name.");
        }

    }

    sprintf(quitMsg, "QUIT %s", quitReason);
    message_send(clientAddr, quitMsg);
    mem_free(quitMsg);
    mem_free(quitReason);

}

/**
 * @brief Sends a message to all clients reflecting the leaderboard when the game ends.
 * 
 * @param game - the game_t struct holding game information
 * @param maxNameLength - the longest a name can be
 */
void
send_gameOverMsg(game_t* game, int maxNameLength)
{
    char* message = mem_malloc_assert(16+ ((maxNameLength + 14) * game->playersJoined), "Error allocating memory in send_gameOverMsg.\n");
    strcpy(message, "QUIT GAME OVER:");

    for (int i = 1; i < game->playersJoined + 1; i++){
        client_t* player = game->clients[i];
        if (player != NULL){
            char* playerStr = mem_malloc_assert(strlen(player->real_name) + 14, "Error allocating memory in send_gameOverMsg.\n");
            sprintf(playerStr, "\n%c       %3d %s", player->id, player->gold, player->real_name);
            strcat(message, playerStr);
            mem_free(playerStr);
        }
    }    

    // Add the null terminator at the end of the message string
    int messageLength = strlen(message);
    message[messageLength] = '\0';

    for (int i = 0; i < game->playersJoined + 1; i++){
        client_t* client = game->clients[i];
        if (client != NULL && !client->quit){
            message_send(client->clientAddr, message);
        }
    }

    mem_free(message);

}