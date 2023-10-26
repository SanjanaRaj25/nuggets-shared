# CS50 Nuggets
## Design Spec
### Team 9: Plankton, Spring, 2023

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server. Since our team has 3 people, we will not be describing or completing the client program.
Our design includes game, grid, server modules, and we make use of certain CS50 libraries including `mem` and `file`.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

### Division of Work
Broadly, we plan to use the scrum framework of work, and work in organized sprints. We will subdivide tasks more specifically in the implementation doc, but we will generally each take charge of:
* Andra: main server functionality and visibility
* Jackson: grid module
* Sanjana: game module

## Server
### User interface

See the requirements spec for the command-line interface.
There is no interaction with the user: the server receives input through messages from the client.

### Inputs and outputs
Inputs:
* Map file: gets passed to grid module to be loaded in as a grid map.

Outputs:
* Messages logged to stderr, which can be redirected to a log file. This includes useful messages, errors as described in the requirements.


### Functional decomposition into modules
* `server.c` - main module, communicates with clients
* `grid.c` - module for handling the global and player grids
* `game.c` - module for handling player and game structures 

### Pseudo code for logic/algorithmic flow

The server will run as follows:

	execute from a command line per the requirement spec
	parse the command line, validate parameters
	call new_game() to set up data structures
	initialize the 'message' module
	print the port number on which we wait
	load grid 
	load gold
	call message_loop(), to await clients
		load a player/spectator for clients
		for each message after a player has been initialized
			handle the message by calling handleMessage()
			call update_clients() to send new map, messages (like new gold counts for instance)
	call gameOver() to inform all clients the game has ended
	clean up

### Function breakdown
#### main
	handle the parsing of parameters and initializing other modules, including messsage

#### handleMessage:
Callback function to be called in message_loop() when a message from a client in received

	parse and validate message string
	if new player
	  if under max players
	     call new_player()
		 call update_grids()
	  else
	    send quit message
	if new spectator
	    call new_spectator()
	if key
	  call handle_keystroke()

		

#### handle_keystroke:
Handle the movement or quitting prompted by a valid key, k. Find the coordinate a player is attempting to move to, and determine if they're allowed to move. If so, call function to move player. If there's gold, handle that accordingly (update player struct and global game status). If there's another player there, handle that accordingly. Then, update the game accordingly.

	Check key
	If quit
	  remove player from map
	  delete_client()
	Check what is in the spot we want to move to
	If boundary
	  Do nothing
	If gold
	  increment player’s purse gold count
	  change grid representation at location to “.”
	  Call update_gold() to decrement count of unclaimed gold
	If player
	  Update global grid to reflect swap

	Call update_grids, which updates all player grids to reflect new global grid

#### gameOver:
	Ends the game and informs all of the clients

#### update_clients:
	Sends a message to all clients with the updated state of the game

#### spectator_quit:
	Sends quit message to spectator


### Major data structures

No major data structures implemented in server specifically, however it relies heavily on the client and game structs implemented in game.c.

## Grid Module

#### The Grid Module will be a collection of helper functions handling grid manipulation to be utilized in the server, it will not have a unique main or algorithmic flow.

### Functional decomposition

* load_grid:
	Create an array of strings, each string is one row of the map. Return the grid.
* grid_toStr:
	Create string version of grid map, containing rows*columns characters and new line characters at the end of every line
* assign_random_spot:
	Assign players and gold to random (open) locations (row, column) on the grid
* update_player_grid:
	Update the player's grid array 
* isVisible:
	For a given player, helps update player_grid to contain points which are currently visible to them
* update_grids:
	Updates all client grids to reflect changes in the server's global grid
* delete_grid:
	Frees all memory associated with a grid

#### Helper functions
	 
* visCol:
	Check columns between player column and boundary column locations for visibility
* visRow:
	Check rows between player row and boundary row locations for visibility
* pcONwc:
	Handle case when the player's column is equal to the boundary's column
	
### Pseudo code for each Function

#### update_player_grid
takes in global and player grids and player's coordinates, establishes wall boundaries, and calls isVisible for each wall point

	initialize player_grid array to global grid

	call helper function check_wall(player coordinates) which will:
		given player's current row, loop through columns above and below until you hit a boundary ("|", "-", "+")
			count number of cells until wall
		given player's current column, loop through rows above and below until you hit a boundary ("|", "-", "+")
			count number of cells until wall
	
	create array of wall points based on max distance to up, down, left, right of player

	loop through wall points array
		for each wall point, call isVisible to verify whether there is full or obstructed visibility between the player and the specific wall point


#### isVisible
takes in player and boundary coordinates and updates player_grid accordingly

	for wall point (wr, wc) in grid array
		for each row exclusively between pr and wr
			compute column coordinates
			if column is an integer (aka a point on grid)
				if grid[row][column] contains a boundary element ("|", "-", "x")
					this and all following row and column points remain inivisible
				else 
					update player_grid[row][column] to value of global grid[row][column]
			else (not a grid point)
				if column element behind or in front is a boundary element
					this and all following row and column points remain invisible
				else
					update player_grid[row][column] to value of global grid[row][column]
		for each column exclusively between pc and wc
			compute row
			repeat row logic above for columns

#### grid_toStr
converts a grid to a string that can be sent to and displayed by the client, takes in player and global grid

	Create string for string version of grid map, must have rows*columns characters plus new lines
	loop thru points in grid
	   sets characters in string to match global grid, unless there is a player grid passed in, who's characters will be matched instead
	return the created string

#### load_grid
takes in a map file and returns a "grid" or an array of strings representing the map

	read map file line by line into array of strings
	return the created grid


### Major data structures

Both global and player-specific grids are created as char** arrays of strings. When passing messages between server and clients, grids are converted to strings.


## Game Module

The Game Module will be a collection of helper functions handling client + game struct manipulation to be utilized by the server, it will not have a unique main or algorithmic flow.

### Functional decomposition

* new_player:
	Adds a new client struct for a player
* new_spectator:
	Adds a new client sturct for a spectator
* delete_client:
	frees memory associated with a client struct, removes it from game struct list of clients
* new_game:
	Initializes game module data structures
* end_game:
	Frees memory associated with game module data structures
* update_gold:
	Updates the amount of gold remaining

### Major data structures

* Client Struct points to the following elements:
	* `const addr_t client` holds the address we can send messages to the client at
	* `bool spectator` if true, the rest of the elements will be set to NULL or 0, keep track of whether the client is a specator.
	* a char, player's ID (a letter between A-Z based on when player joined the game) 
	* player's real name [char array]
	* player's amount of gold [integer value]
	* player's map grid of what they see[array of strings]

* Game struct points to the following elements:
	* the char** global_grid with the entire map
	* an array of client structs that have joined the game
	* an integer keeping track of the amount of gold remaining
	* an integer keeping track of the amount of players that have joined
	* a boolean spectator_active keeping track of whether there is a spectator
	* an int number of rows in the global grid
	* an int number of columns in the global grid
	
## Testing Plan

We plan to create a unit test for the grid module, testing loading in a map file and the functions in the grid module. Additionally we will integration test server, testing that it works with grid and game properly. Game is too small to require a unit test, but a unit test could be beneficial on server to ensure that it can send and receive messages/strings to clients properly. Due to the nature of the program and the graphical user output, regression testing can easily be performed by observing output.
		

