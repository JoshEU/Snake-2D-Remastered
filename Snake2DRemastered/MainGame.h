#pragma once
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

// Contains the max score for the game
static int maxScore = 30;

class MainGame {
public:
	// Set the size and resolution of the game window
	static const int DISPLAY_WIDTH = 1500;
	static const int DISPLAY_HEIGHT = 750;
	static const int DISPLAY_SCALE = 1;
	// Defines all different types of GameObjects in the game
	enum GameObjectType {
		TYPE_NULL = -1,
		TYPE_MOUSEPTR,
		TYPE_SPEEDTICKBOXUI,
		TYPE_WRAPTICKBOXUI,
		TYPE_SNAKEHEADRED,
		TYPE_SNAKEHEADBLUE,
		TYPE_SNAKETAILRED,
		TYPE_SNAKETAILBLUE,
		TYPE_ITEMRED,
		TYPE_ITEMBLUE,
		TYPE_P1CONTROLPROMPT,
		TYPE_P2CONTROLPROMPT,
	};
};