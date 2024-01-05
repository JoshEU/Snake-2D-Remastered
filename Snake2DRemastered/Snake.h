#pragma once
#include "MainGame.h"

class Snake {
public:
	// This Struct will keep track of the snakes property variables.
	struct SnakeProperties {
		// Determines the speed of the snake - changes based on the game speed (set inside the options menu)
		// This integer determines how long it takes before the snake moves measured in ms
		// Lower value = Faster Speed
		// Higher value = Slower Speed
		int snakeSpeed = 30;
		// Stores Player 1's snakes current direction as a float.
		float p1CurrentDirection = PLAY_PI * 1.5f;
		// Stores Player 2's snakes current direction as a float.
		float p2CurrentDirection = PLAY_PI / 2;
		// Starting Positions of both snakes
		Vector2D p1StartPos = { 248 , 372 };
		Vector2D p2StartPos = { 1248 , 372 };
		// Player 1 Snake tail
		int p1TailNumber = 2;
		// Player 2 Snake tail
		int p2TailNumber = 2;
	};
};