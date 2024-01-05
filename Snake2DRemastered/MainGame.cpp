#define PLAY_IMPLEMENTATION
#define VK_W 0x57
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44
#include "MainGame.h"
#include "MainMenu.h"
#include "Snake.h"
#include "GameOver.h"

enum class GameState {
	STATE_MAINMENU = 0,
	STATE_OPTIONMENU,
	STATE_INGAME,
	STATE_ISPLAYING,
	STATE_GAMEOVER,
};

enum class PlayerOneInputType {
	W,
	A,
	S,
	D,
};

enum class  PlayerTwoInputType{
	UP,
	LEFT,
	DOWN,
	RIGHT
};

// Global enum variables:
GameState currentGameState = GameState::STATE_MAINMENU;
PlayerOneInputType playerOneInput;
PlayerTwoInputType playerTwoInput;

// Global struct variables:
Snake::SnakeProperties snakeProperties;

// Global variables:
const int cellWidth = 60;
const int cellHeight = 56;
int checkBoxEnabled = 1;
bool isScreenWrapEnabled = false;
bool p1Win = false;
bool p2Win = false;
bool isPlaying = false;
clock_t timer;
clock_t delayTimer; // This variable delays the appearance of the GameOver screen -> it increments by 1 every frame once STATE_GAMEOVER equals true
Point2f mousePtr;
// Stores the 'x' value for the center of each grid cell along a row within the red arena
float redCenterCellsX[10] = { 128, 188, 248, 308, 368, 428, 488, 548, 608, 668 };
// Stores the 'y' value for the center of each grid cell along a column within the red arena
float redCenterCellsY[10] = { 148, 204, 260, 316, 372, 428, 484, 540, 596, 652 };
float redRandX = 0;
float redRandY = 0;
// Stores the 'x' value for the center of each grid cell along a row within the blue arena
float blueCenterCellsX[10] = { 828, 888, 948, 1008, 1068, 1128, 1188, 1248, 1308, 1368 };
// Stores the 'y' value for the center of each grid cell along a column within the blue arena
float blueCenterCellsY[10] = { 148, 204, 260, 316, 372, 428, 484, 540, 596, 652 };
float blueRandX = 0;
float blueRandY = 0;

// Function Declarations:
// General functions:
void CreateGameObjects();
void DestroyGameObjects();
void CheckGameState();
void ChangeMousePtr();
void ClickMousePtr();
// Main Menu & Option Menu functions:
void DrawUITickBoxes();
void StartButton();
void OptionsButton();
void QuitButton();
void BackButton();
// Main Game functions:
void IncrementTimer();
void ResetTimer();
void DrawGameObjects(); 
void PlayerControls();
void MoveP1Tail();
void MoveP1Snake();
void MoveP2Tail();
void MoveP2Snake();
void CheckScore();
void DetermineWinner();
void CheckBoundaryCollision();
void CheckTailCollision();
void RedRandFoodSpawn();
void BlueRandFoodSpawn();
void SpawnItemP1();
void SpawnItemP2();
void InstantiateRedTail();
void InstantiateBlueTail();
void EatItem();
// GameOver functions:
void RestartButton();
void MainMenuButton();

// The entry point for the game.
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE) {
	Play::CreateManager(MainGame::DISPLAY_WIDTH, MainGame::DISPLAY_HEIGHT, MainGame::DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::StartAudioLoop("snake_menuMusic");
	Play::LoadBackground("Data\\Backgrounds\\SnakeGrid.png");
	Play::LoadBackground("Data\\Backgrounds\\MainMenu.png");
	Play::LoadBackground("Data\\Backgrounds\\OptionsMenu.png");
	Play::LoadBackground("Data\\Backgrounds\\GameOverP1Win.png");
	Play::LoadBackground("Data\\Backgrounds\\GameOverP2Win.png");
	Play::LoadBackground("Data\\Backgrounds\\GameOverTie.png");
	ShowCursor(false);
	CreateGameObjects();
	// Seeds the random number using the current time as a starting condition for the RNG
	// Used for randomly spawning food in the centre of a grid cell when eaten, chooses from an array of predefined co-ordinates
	srand((unsigned int)time(NULL));
}

// Called by PlayBuffer every frame (60 times a second)
bool MainGameUpdate(float elapsedTime) {
	RedRandFoodSpawn();
	BlueRandFoodSpawn();
	CheckGameState();
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

// Gets called once, when the player quits the game. 
int MainGameExit(void) {
	Play::DestroyManager();
	return PLAY_OK;
}

// This Function acts as a gamestate machine by checking what the current state of the game is and executing many functions based on that - called inside MainGameUpdate()
void CheckGameState() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);

	switch (currentGameState) {
	case GameState::STATE_MAINMENU:
		Play::DrawBackground(1);
		break;
	case GameState::STATE_OPTIONMENU:
		Play::DrawBackground(2);
		DrawUITickBoxes();
		break;
	case GameState::STATE_INGAME:
		// Display Game stuff
		Play::DrawBackground(0);
		DrawGameObjects();
		// Checking for any player input in order to start the gameplay
		PlayerControls();
		// Start the gameplay
		if (isPlaying) {
			IncrementTimer();
			if (timer > snakeProperties.snakeSpeed) { 
				MoveP1Snake();
				MoveP1Tail();
				MoveP2Snake();
				MoveP2Tail();
			}
			CheckBoundaryCollision();
			CheckTailCollision();
			EatItem();
		}
		break;
	case GameState::STATE_GAMEOVER:
		isPlaying = false;
		Play::StopAudioLoop("snake_gameMusic");
		// Initialize P1 Snake direction to default value for when the game is restarted
		snakeProperties.p1CurrentDirection = PLAY_PI * 1.5f;
		// Initialize P2 Snake direction to default value for when the game is restarted
		snakeProperties.p2CurrentDirection = PLAY_PI / 2;
		// Initialize P1 snake tail number back to default value
		snakeProperties.p1TailNumber = 2;
		// Initialize P2 snake tail number back to default value
		snakeProperties.p2TailNumber = 2;
		// This variable delays the appearance of the GameOver screen -> it increments by 1 every frame once STATE_GAMEOVER equals true
		delayTimer += 1;
		// Set Losing Snakes head to Dead Sprite
		// Separate if statements so it's possible for both snakes to be dead simultaneously
		if (p1Win) {
			// Kill P2 Snake
			obj_snakeHeadBlue.pos = obj_snakeHeadBlue.oldPos;
			Play::SetSprite(obj_snakeHeadBlue, "snakeHeadBlueDead", 0.0f);

			Play::UpdateGameObject(obj_snakeHeadBlue);
			Play::DrawObjectRotated(obj_snakeHeadBlue, 1.0f);
		}
		if (p2Win) {
			// Kill P1 Snake
			obj_snakeHeadRed.pos = obj_snakeHeadRed.oldPos;
			Play::SetSprite(obj_snakeHeadRed, "snakeHeadRedDead", 0.0f);
		
			Play::UpdateGameObject(obj_snakeHeadRed);
			Play::DrawObjectRotated(obj_snakeHeadRed, 1.0f);
		}
		// Display P1 Win GameOver Background after delay timer reaches max threshold
		if (delayTimer >= 300 && p1Win && p2Win == false) {
			DestroyGameObjects();
			Play::DrawBackground(3);
			// Make MousePtr visible again as it was 'TYPE_NULL' during gameplay
			Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_MOUSEPTR).type = MainGame::GameObjectType::TYPE_MOUSEPTR;
			// Re-create the starting GameObjects
			CreateGameObjects();
		}
		// Display P2 Win GameOver Background after delay timer reaches max threshold
		if (delayTimer >= 300 && p2Win && p1Win == false) {
			DestroyGameObjects();
			Play::DrawBackground(4);
			// Make MousePtr visible again as it was 'TYPE_NULL' during gameplay
			Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_MOUSEPTR).type = MainGame::GameObjectType::TYPE_MOUSEPTR;
			// Re-create the starting GameObjects
			CreateGameObjects();
		}
		// Display Tie GameOver Background after delay timer reaches max threshold
		if (delayTimer >= 750 && p1Win && p2Win) {
			 DestroyGameObjects();
			 Play::DrawBackground(5);
			 // Make MousePtr visible again as it was 'TYPE_NULL' during gameplay
			 Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_MOUSEPTR).type = MainGame::GameObjectType::TYPE_MOUSEPTR;
			 // Re-create the starting GameObjects
			 CreateGameObjects();
		}
		break;
	default:
		break;
	}
	ChangeMousePtr();
	ClickMousePtr();
}

// This Function creates the neccessary objects required to begin the game - called inside MainGameEntry() & CheckGameState()
void CreateGameObjects() {
	// Creating GameObjects:
	int id_mousePtr = Play::CreateGameObject(MainGame::GameObjectType::TYPE_MOUSEPTR, Play::GetMousePos(), 10, "redBlueMousePtr");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SPEEDTICKBOXUI, { 0 , 0 }, 10, "tickBoxUI");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_WRAPTICKBOXUI, { MainMenu::screenWrapCheckBoxMaxX - 15 , MainMenu::screenWrapCheckBoxMaxY - 14 }, 10, "tickBoxUI");
	// Create Starting Snake Head and Tail GameObjects:
	// Red Snake:
	int id_snakeHeadRed = Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKEHEADRED, snakeProperties.p1StartPos, 25, "snakeHeadRed");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKETAILRED, { 188 , 372 }, 25, "snakeTailRed");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKETAILRED, { 128 , 372 }, 25, "snakeTailRed");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_ITEMRED, { 488 , 372 }, 15, "itemRed");
	// Blue Snake:
	int id_snakeHeadBlue = Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE, snakeProperties.p2StartPos, 25, "snakeHeadBlue");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKETAILBLUE, { 1311 , 372 }, 25, "snakeTailBlue");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKETAILBLUE, { 1371 , 372 }, 25, "snakeTailBlue");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_ITEMBLUE, { 1008, 372 }, 15, "itemBlue");
	// Create Control Prompt Sprites
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_P1CONTROLPROMPT, { 397, 235}, 10, "p1ControlPrompt");
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_P2CONTROLPROMPT, { 1095, 220 }, 10, "p2ControlPrompt"); 
	// Altering GameObjects properties
	Play::GetGameObject(id_mousePtr).pos = Play::GetMousePos();
	Play::GetGameObject(id_snakeHeadRed).pos = snakeProperties.p1StartPos;
	Play::GetGameObject(id_snakeHeadRed).rotation = snakeProperties.p1CurrentDirection;
	Play::GetGameObject(id_snakeHeadBlue).pos = snakeProperties.p2StartPos;
	Play::GetGameObject(id_snakeHeadBlue).rotation = snakeProperties.p2CurrentDirection;
}

// This Function draws the gameobjects that are in the game and updates them - called inside CheckGameState()
void DrawGameObjects() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	GameObject& obj_itemRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_ITEMRED);
	GameObject& obj_itemBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_ITEMBLUE);
	GameObject& obj_p1ControlPrompt = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_P1CONTROLPROMPT);
	GameObject& obj_p2ControlPrompt = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_P2CONTROLPROMPT);
	std::vector<int> v_P1Tails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILRED);
	std::vector<int> v_P2Tails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILBLUE);

	for (int id : v_P1Tails) {
		GameObject& obj_redTails = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_redTails);
		Play::DrawObject(obj_redTails);
	}
	for (int id : v_P2Tails) {
		GameObject& obj_blueTails = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_blueTails);
		Play::DrawObject(obj_blueTails);
	}
	Play::UpdateGameObject(obj_snakeHeadRed);
	Play::UpdateGameObject(obj_snakeHeadBlue);
	Play::UpdateGameObject(obj_itemRed);
	Play::UpdateGameObject(obj_itemBlue);

	Play::DrawObjectRotated(obj_snakeHeadRed, 1.0f);
	Play::DrawObjectRotated(obj_snakeHeadBlue, 1.0f);
	Play::DrawObject(obj_itemRed);
	Play::DrawObject(obj_itemBlue);

	// Draws Control Prompts for both players at the start of the game prior to any movement occuring
	Play::DrawObject(obj_p1ControlPrompt);
	Play::DrawObject(obj_p2ControlPrompt);

	// Draws Player 1 score to screen
	Play::DrawFontText("64px", std::to_string(snakeProperties.p1TailNumber - 2),
		{ 155, 722 }, Play::CENTRE);

	// Draws Player 2 score to screen
	Play::DrawFontText("64px", std::to_string(snakeProperties.p2TailNumber - 2),
		{ 855, 722 }, Play::CENTRE);
	CheckScore();
}

// This Function destroys all gameobjects at the end of the game - called inside CheckGameState()
void DestroyGameObjects() {
	// Destroy all GameObjects
	for (int id_allObjs : Play::CollectAllGameObjectIDs()) {
		Play::DestroyGameObject(id_allObjs);
	}
}

// This Function changes the sprite of the mouse pointer based on whether or not its hovering over a UI element inside the game - called inside CheckGameState()
void ChangeMousePtr() {
	GameObject& obj_mousePtr = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_MOUSEPTR);
	// Sets GameObjects position to mouse position
	obj_mousePtr.pos = Play::GetMousePos();
	// Stores the mousePtr GameObjects position inside a Point2f variable
	mousePtr = obj_mousePtr.pos;

	// Check if Mouse Pointer is hovering over Start Button OR Options Button OR Quit Button inside the Main Menu OR Any UI elements in the game.
	if ( /* Main Menu Buttons */ currentGameState == GameState::STATE_MAINMENU && mousePtr.x >= MainMenu::startBtnMinX && mousePtr.x <= MainMenu::startBtnMaxX && mousePtr.y >= MainMenu::startBtnMinY && mousePtr.y <= MainMenu::startBtnMaxY || currentGameState == GameState::STATE_MAINMENU && mousePtr.x >= MainMenu::optionBtnMinX && mousePtr.x <= MainMenu::optionBtnMaxX && mousePtr.y >= MainMenu::optionBtnMinY && mousePtr.y <= MainMenu::optionBtnMaxY || currentGameState == GameState::STATE_MAINMENU && mousePtr.x >= MainMenu::quitBtnMinX && mousePtr.x <= MainMenu::quitBtnMaxX && mousePtr.y >= MainMenu::quitBtnMinY && mousePtr.y <= MainMenu::quitBtnMaxY || /* Option Menu Buttons/UI Elements */ currentGameState == GameState::STATE_OPTIONMENU && mousePtr.x >= MainMenu::backBtnMinX && mousePtr.x <= MainMenu::backBtnMaxX && mousePtr.y >= MainMenu::backBtnMinY && mousePtr.y <= MainMenu::backBtnMaxY || currentGameState == GameState::STATE_OPTIONMENU && mousePtr.x >= MainMenu::oneCheckBoxMinX && mousePtr.x <= MainMenu::oneCheckBoxMaxX && mousePtr.y >= MainMenu::oneCheckBoxMinY && mousePtr.y <= MainMenu::oneCheckBoxMaxY || currentGameState == GameState::STATE_OPTIONMENU && mousePtr.x >= MainMenu::twoCheckBoxMinX && mousePtr.x <= MainMenu::twoCheckBoxMaxX && mousePtr.y >= MainMenu::twoCheckBoxMinY && mousePtr.y <= MainMenu::twoCheckBoxMaxY || currentGameState == GameState::STATE_OPTIONMENU && mousePtr.x >= MainMenu::threeCheckBoxMinX && mousePtr.x <= MainMenu::threeCheckBoxMaxX && mousePtr.y >= MainMenu::threeCheckBoxMinY && mousePtr.y <= MainMenu::threeCheckBoxMaxY || currentGameState == GameState::STATE_OPTIONMENU && mousePtr.x >= MainMenu::screenWrapCheckBoxMinX && mousePtr.x <= MainMenu::screenWrapCheckBoxMaxX && mousePtr.y >= MainMenu::screenWrapCheckBoxMinY && mousePtr.y <= MainMenu::screenWrapCheckBoxMaxY || /* GameOver Screen Buttons */ currentGameState == GameState::STATE_GAMEOVER && mousePtr.x >=  GameOver::restartBtnMinX && mousePtr.x <= GameOver::restartBtnMaxX && mousePtr.y >= GameOver::restartBtnMinY && mousePtr.y <= GameOver::restartBtnMaxY || currentGameState == GameState::STATE_GAMEOVER && mousePtr.x >= GameOver::mainMenuBtnMinX && mousePtr.x <= GameOver::mainMenuBtnMaxX && mousePtr.y >= GameOver::mainMenuBtnMinY && mousePtr.y <= GameOver::mainMenuBtnMaxY || currentGameState == GameState::STATE_GAMEOVER && mousePtr.x >= GameOver::quitBtnMinX && mousePtr.x <= GameOver::quitBtnMaxX && mousePtr.y >= GameOver::quitBtnMinY && mousePtr.y <= GameOver::quitBtnMaxY) {
		// Change Mouse Pointer sprite to hand
		Play::SetSprite(obj_mousePtr, "redBlueHandPtr", 0);
	}
	else {
		// Change Mouse Pointer sprite back to original 
		Play::SetSprite(obj_mousePtr, "redBlueMousePtr", 0);
	}

	// Only switch the mouse pointer sprite in the Main Menu and GameOver Screen
	// Make mouse pointer invisible during gameplay
	if (currentGameState != GameState::STATE_MAINMENU && currentGameState != GameState::STATE_OPTIONMENU && currentGameState != GameState::STATE_GAMEOVER) {
		// Change Mouse Pointer sprite back to original
		Play::SetSprite(obj_mousePtr, "redBlueMousePtr", 0);
		Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_MOUSEPTR).type = MainGame::GameObjectType::TYPE_NULL;
	}

	Play::UpdateGameObject(obj_mousePtr);
	// Draws the custom mouse pointer to the display
	// Not used in DrawGameObjects() function as that is only called when the currentGameState is set to 'STATE_INGAME'
	Play::DrawObject(obj_mousePtr);
}

// This Function contains the input detection and functionality for the user pressing a UI element inside the game - called inside CheckGameState()
void ClickMousePtr() {
	GameObject& obj_mousePtr = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_MOUSEPTR);
	// Sets GameObjects position to mouse position
	obj_mousePtr.pos = Play::GetMousePos();
	// Check for User Input on the Main Menu & Option Menu & Game Over screen Buttons 
	if (Play::KeyPressed(VK_LBUTTON)) {
		// Main Menu Start Button check
		if (mousePtr.x >= MainMenu::startBtnMinX && mousePtr.x <= MainMenu::startBtnMaxX && mousePtr.y >= MainMenu::startBtnMinY && mousePtr.y <= MainMenu::startBtnMaxY && currentGameState == GameState::STATE_MAINMENU) {
			StartButton();
		}
		// Main Menu Option Button check
		else if (mousePtr.x >= MainMenu::optionBtnMinX && mousePtr.x <= MainMenu::optionBtnMaxX && mousePtr.y >= MainMenu::optionBtnMinY && mousePtr.y <= MainMenu::optionBtnMaxY && currentGameState == GameState::STATE_MAINMENU) {
			OptionsButton();
		}
		// Main Menu Quit Button check
		else if (mousePtr.x >= MainMenu::quitBtnMinX && mousePtr.x <= MainMenu::quitBtnMaxX && mousePtr.y >= MainMenu::quitBtnMinY && mousePtr.y <= MainMenu::quitBtnMaxY && currentGameState == GameState::STATE_MAINMENU) {
			QuitButton();
		}
		// Option Menu Back Button check
		else if (mousePtr.x >= MainMenu::backBtnMinX && mousePtr.x <= MainMenu::backBtnMaxX && mousePtr.y >= MainMenu::backBtnMinY && mousePtr.y <= MainMenu::backBtnMaxY && currentGameState == GameState::STATE_OPTIONMENU) {
			BackButton();
		}
		// Option Menu Game Speed 1st Checkbox
		else if (mousePtr.x >= MainMenu::oneCheckBoxMinX && mousePtr.x <= MainMenu::oneCheckBoxMaxX && mousePtr.y >= MainMenu::oneCheckBoxMinY && mousePtr.y <= MainMenu::oneCheckBoxMaxY && currentGameState == GameState::STATE_OPTIONMENU) {
			checkBoxEnabled = 1;
		}
		// Option Menu Game Speed 2nd Checkbox
		else if (mousePtr.x >= MainMenu::twoCheckBoxMinX && mousePtr.x <= MainMenu::twoCheckBoxMaxX && mousePtr.y >= MainMenu::twoCheckBoxMinY && mousePtr.y <= MainMenu::twoCheckBoxMaxY && currentGameState == GameState::STATE_OPTIONMENU) {
			checkBoxEnabled = 2;
		}
		// Option Menu Game Speed 3rd Checkbox
		else if (mousePtr.x >= MainMenu::threeCheckBoxMinX && mousePtr.x <= MainMenu::threeCheckBoxMaxX && mousePtr.y >= MainMenu::threeCheckBoxMinY && mousePtr.y <= MainMenu::threeCheckBoxMaxY && currentGameState == GameState::STATE_OPTIONMENU) {
			checkBoxEnabled = 3;
		}
		// Option Menu Screen Wrap CheckBox Enable
		else if (mousePtr.x >= MainMenu::screenWrapCheckBoxMinX && mousePtr.x <= MainMenu::screenWrapCheckBoxMaxX && mousePtr.y >= MainMenu::screenWrapCheckBoxMinY && mousePtr.y <= MainMenu::screenWrapCheckBoxMaxY && currentGameState == GameState::STATE_OPTIONMENU && isScreenWrapEnabled == false) {
			isScreenWrapEnabled = true;
		}
		// Option Menu Screen Wrap CheckBox Disable
		else if (mousePtr.x >= MainMenu::screenWrapCheckBoxMinX && mousePtr.x <= MainMenu::screenWrapCheckBoxMaxX && mousePtr.y >= MainMenu::screenWrapCheckBoxMinY && mousePtr.y <= MainMenu::screenWrapCheckBoxMaxY && currentGameState == GameState::STATE_OPTIONMENU && isScreenWrapEnabled == true) {
			isScreenWrapEnabled = false;
		}
		// GameOver Screen Restart Button Check
		else if (mousePtr.x >= GameOver::restartBtnMinX && mousePtr.x <= GameOver::restartBtnMaxX && mousePtr.y >= GameOver::restartBtnMinY && mousePtr.y <= GameOver::restartBtnMaxY && currentGameState == GameState::STATE_GAMEOVER) {
			RestartButton();
		}
		// GameOver Screen Main Menu Button Check
		else if (mousePtr.x >= GameOver::mainMenuBtnMinX && mousePtr.x <= GameOver::mainMenuBtnMaxX && mousePtr.y >= GameOver::mainMenuBtnMinY && mousePtr.y <= GameOver::mainMenuBtnMaxY && currentGameState == GameState::STATE_GAMEOVER) {
			MainMenuButton();
		}
		// GameOver Screen Quit Button Check
		else if (mousePtr.x >= GameOver::quitBtnMinX && mousePtr.x <= GameOver::quitBtnMaxX && mousePtr.y >= GameOver::quitBtnMinY && mousePtr.y <= GameOver::quitBtnMaxY && currentGameState == GameState::STATE_GAMEOVER) {
			QuitButton();
		}
	}
	Play::UpdateGameObject(obj_mousePtr);
	// Draws the custom mouse pointer to the display
	// Not used in DrawGameObjects() function as that is only called when the currentGameState is set to 'STATE_INGAME'
	Play::DrawObject(obj_mousePtr);
}

// This Function includes the Drawing of the Option Menu's checkboxes - called inside CheckGameState()
// Draws a tick sprite according to which checkbox is selected
void DrawUITickBoxes() {
	GameObject& obj_speedTickBoxUI = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SPEEDTICKBOXUI);
	GameObject& obj_wrapTickBoxUI = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_WRAPTICKBOXUI);
	// Ticks the game speed check box according to which one the user clicked on with the mouse pointer
	// Default is 1
	switch (checkBoxEnabled) {
	case 1:
		obj_speedTickBoxUI.pos = { MainMenu::oneCheckBoxMaxX - 15, MainMenu::oneCheckBoxMaxY - 14 };
		// Slow Speed - High Number
		snakeProperties.snakeSpeed = 30;
		break;
	case 2:
		obj_speedTickBoxUI.pos = { MainMenu::twoCheckBoxMaxX - 15, MainMenu::twoCheckBoxMaxY -14 };
		// Medium Speed - Middle Number
		snakeProperties.snakeSpeed = 20;
		break;
	case 3:
		obj_speedTickBoxUI.pos = { MainMenu::threeCheckBoxMaxX - 15, MainMenu::threeCheckBoxMaxY - 14 };
		// Fast Speed - Low Number
		snakeProperties.snakeSpeed = 10;
		break;
	default:
		break;
	}
	// Ticks/Un-ticks the Screen Wrapping checkbox according to user input
	switch (isScreenWrapEnabled) {
	case true:
		Play::UpdateGameObject(obj_wrapTickBoxUI);
		Play::DrawObject(obj_wrapTickBoxUI);
		break;
	case false:
		// Do Nothing 
		break;
	}
	Play::UpdateGameObject(obj_speedTickBoxUI);
	Play::DrawObject(obj_speedTickBoxUI);
}

// This Function includes the Start Button functionality for the Main Menu - called inside ClickMousePtr()
// Loads the Main Game Background
void StartButton() {
	// Reset both win bools to false when the restart button is clicked
	p1Win = false;
	p2Win = false;
	// Set state back to 'STATE_INGAME'
	currentGameState = GameState::STATE_INGAME;
	Play::StopAudioLoop("snake_menuMusic");
	Play::StartAudioLoop("snake_gameMusic");
}

// This Function includes the Option Button functionality for the Main Menu - called inside ClickMousePtr()
// Loads the Option Menu Background
void OptionsButton() {
	// Perform Options functionality
	currentGameState = GameState::STATE_OPTIONMENU;
}

// This Function includes the Quit Button functionality for the Main Menu - called inside ClickMousePtr()
// Quits the .exe
void QuitButton() {
	exit(0);
}

// This Function includes the Back Button functionality for the Options Menu - called inside ClickMousePtr()
// Returns to Main Menu from the Options Menu
void BackButton() {
	currentGameState = GameState::STATE_MAINMENU;
}

// This Function includes the Restart Button functionality for the GameOver Screen - called inside ClickMousePtr()
void RestartButton() {
	// Reset both win bools to false when the restart button is clicked
	p1Win = false;
	p2Win = false;
	// Set state back to 'STATE_INGAME'
	currentGameState = GameState::STATE_INGAME;
	Play::StartAudioLoop("snake_gameMusic");
	delayTimer = 0;
}

// This Function includes the Back to Main Menu Button functionality for the GameOver Screen - called inside ClickMousePtr()
void MainMenuButton() {
	currentGameState = GameState::STATE_MAINMENU;
	delayTimer = 0;
	Play::StartAudioLoop("snake_menuMusic");
}

// Increments timer by 1 - called inside CheckGameState()
void IncrementTimer() {
	timer += 1;
}

//	Resets timer to 0 - called inside CheckGameState()
void ResetTimer() {
	timer = 0;
}

// This Function consists of detecting Player Input and moving the snake accordingly - called inside CheckGameState()
void PlayerControls() {	
	GameObject& obj_p1ControlPrompt = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_P1CONTROLPROMPT);
	GameObject& obj_p2ControlPrompt = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_P2CONTROLPROMPT);

	// PLAYER 1 INPUT DETECTION:
	// Checks for player input on 'WASD' keys:
	// Checks if W key is pressed & if player 1's snake isn't currently facing in the South direction (Downward direction).  
	if (Play::KeyDown(VK_W) && snakeProperties.p1CurrentDirection != 0 && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerOneInput = PlayerOneInputType::W;
	}
	// Checks if A key is pressed & if player 1's snake isn't currently facing in the East direction (Right direction).
	else if (Play::KeyDown(VK_A) && snakeProperties.p1CurrentDirection != PLAY_PI * 1.5f && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerOneInput = PlayerOneInputType::A;
	}
	// Checks if S key is pressed & if player 1's snake isn't currently facing in the North direction (Upward direction).
	else if (Play::KeyDown(VK_S) && snakeProperties.p1CurrentDirection != PLAY_PI && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerOneInput = PlayerOneInputType::S;
	}
	// Checks if D key is pressed & if player 1's snake isn't currently facing in the West direction (Left direction).
	else if (Play::KeyDown(VK_D) && snakeProperties.p1CurrentDirection != PLAY_PI / 2 && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerOneInput = PlayerOneInputType::D;
	}
	else if (isPlaying == false) {
		// Set default direction for P1 snake to be East (Right) 
		// This is in the case that P2's Snake moves first - then P1 will begin moving in their default direction
		playerOneInput = PlayerOneInputType::D;
	}
	// PLAYER 2 INPUT DETECTION:
	// Checks for player input on 'UP,LEFT,DOWN,RIGHT' arrow keys:	
	// Checks if UP arrow key is pressed & if player 2's snake isn't currently facing in the South direction (Downward direction).
	if (Play::KeyDown(VK_UP) && snakeProperties.p2CurrentDirection != 0 && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerTwoInput = PlayerTwoInputType::UP;
	}
	// Checks if LEFT arrow key is pressed & if player 2's snake isn't currently facing in the East direction (Right direction).
	else if (Play::KeyDown(VK_LEFT) && snakeProperties.p2CurrentDirection != PLAY_PI * 1.5f && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerTwoInput = PlayerTwoInputType::LEFT;
	}
	// Checks if DOWN arrow key is pressed & if player 2's snake isn't currently facing in the North direction (Upward direction).
	else if (Play::KeyDown(VK_DOWN) && snakeProperties.p2CurrentDirection != PLAY_PI && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerTwoInput = PlayerTwoInputType::DOWN;
	}
	// Checks if RIGHT arrow key is pressed & if player 2's snake isn't currently facing in the West direction (Left direction).
	else if (Play::KeyDown(VK_RIGHT) && snakeProperties.p2CurrentDirection != PLAY_PI / 2 && currentGameState != GameState::STATE_GAMEOVER) {
		isPlaying = true;
		playerTwoInput = PlayerTwoInputType::RIGHT;
	}
	else if (isPlaying == false) {
		// Set default direction for P2 snake to be West (Left)
		// This is in the case that P1's Snake moves first - then P2 will begin moving in their default direction
		playerTwoInput = PlayerTwoInputType::LEFT;
	}
	// Remove control prompt images from screen when the gameplay commences
	if (isPlaying == true) {
		Play::SetSprite(obj_p1ControlPrompt, "imageNull", 0.0f);
		Play::SetSprite(obj_p2ControlPrompt, "imageNull", 0.0f);
	}
	// There is no checks for determining whether or not the user is giving any input as I want the snake to be constantly moving.
}

// This Function moves Player 1's snakes tail according to the previous position of the tail segment in front of it - called in CheckGameState()
void MoveP1Tail(){
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	std::vector<int> v_redTails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILRED);

	//Initialize the max size of a vector data structure containing snake red tail id's.
	v_redTails.resize(999);

	// Retrieve each element from the vector and assign a GameObject to each one
	GameObject& obj_redTail1 = Play::GetGameObject(v_redTails[0]);
	GameObject& obj_redTail2 = Play::GetGameObject(v_redTails[1]);
	GameObject& obj_redTail3 = Play::GetGameObject(v_redTails[2]);
	GameObject& obj_redTail4 = Play::GetGameObject(v_redTails[3]);
	GameObject& obj_redTail5 = Play::GetGameObject(v_redTails[4]);
	GameObject& obj_redTail6 = Play::GetGameObject(v_redTails[5]);
	GameObject& obj_redTail7 = Play::GetGameObject(v_redTails[6]);
	GameObject& obj_redTail8 = Play::GetGameObject(v_redTails[7]);
	GameObject& obj_redTail9 = Play::GetGameObject(v_redTails[8]);
	GameObject& obj_redTail10 = Play::GetGameObject(v_redTails[9]);
	GameObject& obj_redTail11 = Play::GetGameObject(v_redTails[10]);
	GameObject& obj_redTail12 = Play::GetGameObject(v_redTails[11]);
	GameObject& obj_redTail13 = Play::GetGameObject(v_redTails[12]);
	GameObject& obj_redTail14 = Play::GetGameObject(v_redTails[13]);
	GameObject& obj_redTail15 = Play::GetGameObject(v_redTails[14]);
	GameObject& obj_redTail16 = Play::GetGameObject(v_redTails[15]);
	GameObject& obj_redTail17 = Play::GetGameObject(v_redTails[16]);
	GameObject& obj_redTail18 = Play::GetGameObject(v_redTails[17]);
	GameObject& obj_redTail19 = Play::GetGameObject(v_redTails[18]);
	GameObject& obj_redTail20 = Play::GetGameObject(v_redTails[19]);
	GameObject& obj_redTail21 = Play::GetGameObject(v_redTails[20]);
	GameObject& obj_redTail22 = Play::GetGameObject(v_redTails[21]);
	GameObject& obj_redTail23 = Play::GetGameObject(v_redTails[22]);
	GameObject& obj_redTail24 = Play::GetGameObject(v_redTails[23]);
	GameObject& obj_redTail25 = Play::GetGameObject(v_redTails[24]);
	GameObject& obj_redTail26 = Play::GetGameObject(v_redTails[25]);
	GameObject& obj_redTail27 = Play::GetGameObject(v_redTails[26]);
	GameObject& obj_redTail28 = Play::GetGameObject(v_redTails[27]);
	GameObject& obj_redTail29 = Play::GetGameObject(v_redTails[28]);
	GameObject& obj_redTail30 = Play::GetGameObject(v_redTails[29]);
	GameObject& obj_redTail31 = Play::GetGameObject(v_redTails[30]);
	GameObject& obj_redTail32 = Play::GetGameObject(v_redTails[31]);

	// Move Tail to previous head position and to previous tail position in the vector
	obj_redTail1.pos = obj_snakeHeadRed.oldPos;
	obj_redTail2.pos = obj_redTail1.oldPos;
	obj_redTail3.pos = obj_redTail2.oldPos;
	obj_redTail4.pos = obj_redTail3.oldPos;
	obj_redTail5.pos = obj_redTail4.oldPos;
	obj_redTail6.pos = obj_redTail5.oldPos;
	obj_redTail7.pos = obj_redTail6.oldPos;
	obj_redTail8.pos = obj_redTail7.oldPos;
	obj_redTail9.pos = obj_redTail8.oldPos;
	obj_redTail10.pos = obj_redTail9.oldPos;
	obj_redTail11.pos = obj_redTail10.oldPos;
	obj_redTail12.pos = obj_redTail11.oldPos;
	obj_redTail13.pos = obj_redTail12.oldPos;
	obj_redTail14.pos = obj_redTail13.oldPos;
	obj_redTail15.pos = obj_redTail14.oldPos;
	obj_redTail16.pos = obj_redTail15.oldPos;
	obj_redTail17.pos = obj_redTail16.oldPos;
	obj_redTail18.pos = obj_redTail17.oldPos;
	obj_redTail19.pos = obj_redTail18.oldPos;
	obj_redTail20.pos = obj_redTail19.oldPos;
	obj_redTail21.pos = obj_redTail20.oldPos;
	obj_redTail22.pos = obj_redTail21.oldPos;
	obj_redTail23.pos = obj_redTail22.oldPos;
	obj_redTail24.pos = obj_redTail23.oldPos;
	obj_redTail25.pos = obj_redTail24.oldPos;
	obj_redTail26.pos = obj_redTail25.oldPos;
	obj_redTail27.pos = obj_redTail26.oldPos;
	obj_redTail28.pos = obj_redTail27.oldPos;
	obj_redTail29.pos = obj_redTail28.oldPos;
	obj_redTail30.pos = obj_redTail29.oldPos;
	obj_redTail31.pos = obj_redTail30.oldPos;
	obj_redTail32.pos = obj_redTail31.oldPos;
}

// This Function consists of moving Player 1's snake according to their input - called in CheckGameState()
// Rotates head of snake according to direction of travel 
void MoveP1Snake() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	if (isPlaying) {
		switch (playerOneInput) { 
		case PlayerOneInputType::W:		
			// Move head up
			obj_snakeHeadRed.pos.y = obj_snakeHeadRed.oldPos.y - cellHeight;
			MoveP1Tail();
			// Sets P1 Snake direction to be the current direction it is facing - so the player cannot travel from up to down, left to right and vice versa.
			snakeProperties.p1CurrentDirection = PLAY_PI;
			// Rotate head to face in the upward direction
			obj_snakeHeadRed.rotation = snakeProperties.p1CurrentDirection;
			ResetTimer();
			break;
		case PlayerOneInputType::A:
			// Move head Left
			obj_snakeHeadRed.pos.x = obj_snakeHeadRed.oldPos.x - cellWidth;
			MoveP1Tail();
			snakeProperties.p1CurrentDirection = PLAY_PI / 2;
			// Rotate head to face in the left direction
			obj_snakeHeadRed.rotation = snakeProperties.p1CurrentDirection;
			ResetTimer();
			break;
		case PlayerOneInputType::S:
			// Move head Down
			obj_snakeHeadRed.pos.y = obj_snakeHeadRed.oldPos.y + cellHeight;
			MoveP1Tail();
			snakeProperties.p1CurrentDirection = 0;
			// Rotate head to face in the downward direction
			obj_snakeHeadRed.rotation = snakeProperties.p1CurrentDirection;
			ResetTimer();
			break;
		case PlayerOneInputType::D:
			// Move head Right
			obj_snakeHeadRed.pos.x = obj_snakeHeadRed.pos.x + cellWidth;
			MoveP1Tail();
			snakeProperties.p1CurrentDirection = PLAY_PI * 1.5f;
			// Rotate head to face in the right direction
			obj_snakeHeadRed.rotation = snakeProperties.p1CurrentDirection;
			ResetTimer();
			break;
		default:
			break;
		}
	}
}

// This Function moves Player 2's snakes tail according to the previous position of the tail segment in front of it - called in CheckGameState()
void MoveP2Tail() {
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	std::vector<int> v_blueTails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILBLUE);

	// Initialize the max size of a vector data structure containing snake blue tail id's.
	v_blueTails.resize(999);

	// Retrieve each element from the vector and assign a GameObject to each one 
	GameObject& obj_blueTail1 = Play::GetGameObject(v_blueTails[0]);
	GameObject& obj_blueTail2 = Play::GetGameObject(v_blueTails[1]);
	GameObject& obj_blueTail3 = Play::GetGameObject(v_blueTails[2]);
	GameObject& obj_blueTail4 = Play::GetGameObject(v_blueTails[3]);
	GameObject& obj_blueTail5 = Play::GetGameObject(v_blueTails[4]);
	GameObject& obj_blueTail6 = Play::GetGameObject(v_blueTails[5]);
	GameObject& obj_blueTail7 = Play::GetGameObject(v_blueTails[6]);
	GameObject& obj_blueTail8 = Play::GetGameObject(v_blueTails[7]);
	GameObject& obj_blueTail9 = Play::GetGameObject(v_blueTails[8]);
	GameObject& obj_blueTail10 = Play::GetGameObject(v_blueTails[9]);
	GameObject& obj_blueTail11 = Play::GetGameObject(v_blueTails[10]);
	GameObject& obj_blueTail12 = Play::GetGameObject(v_blueTails[11]);
	GameObject& obj_blueTail13 = Play::GetGameObject(v_blueTails[12]);
	GameObject& obj_blueTail14 = Play::GetGameObject(v_blueTails[13]);
	GameObject& obj_blueTail15 = Play::GetGameObject(v_blueTails[14]);
	GameObject& obj_blueTail16 = Play::GetGameObject(v_blueTails[15]);
	GameObject& obj_blueTail17 = Play::GetGameObject(v_blueTails[16]);
	GameObject& obj_blueTail18 = Play::GetGameObject(v_blueTails[17]);
	GameObject& obj_blueTail19 = Play::GetGameObject(v_blueTails[18]);
	GameObject& obj_blueTail20 = Play::GetGameObject(v_blueTails[19]);
	GameObject& obj_blueTail21 = Play::GetGameObject(v_blueTails[20]);
	GameObject& obj_blueTail22 = Play::GetGameObject(v_blueTails[21]);
	GameObject& obj_blueTail23 = Play::GetGameObject(v_blueTails[22]);
	GameObject& obj_blueTail24 = Play::GetGameObject(v_blueTails[23]);
	GameObject& obj_blueTail25 = Play::GetGameObject(v_blueTails[24]);
	GameObject& obj_blueTail26 = Play::GetGameObject(v_blueTails[25]);
	GameObject& obj_blueTail27 = Play::GetGameObject(v_blueTails[26]);
	GameObject& obj_blueTail28 = Play::GetGameObject(v_blueTails[27]);
	GameObject& obj_blueTail29 = Play::GetGameObject(v_blueTails[28]);
	GameObject& obj_blueTail30 = Play::GetGameObject(v_blueTails[29]);
	GameObject& obj_blueTail31 = Play::GetGameObject(v_blueTails[30]);
	GameObject& obj_blueTail32 = Play::GetGameObject(v_blueTails[31]);

	// Move Tail to previous head position and to previous tail position in the vector
	obj_blueTail1.pos = obj_snakeHeadBlue.oldPos;
	obj_blueTail2.pos = obj_blueTail1.oldPos;
	obj_blueTail3.pos = obj_blueTail2.oldPos;
	obj_blueTail4.pos = obj_blueTail3.oldPos;
	obj_blueTail5.pos = obj_blueTail4.oldPos;
	obj_blueTail6.pos = obj_blueTail5.oldPos;
	obj_blueTail7.pos = obj_blueTail6.oldPos;
	obj_blueTail8.pos = obj_blueTail7.oldPos;
	obj_blueTail9.pos = obj_blueTail8.oldPos;
	obj_blueTail10.pos = obj_blueTail9.oldPos;
	obj_blueTail11.pos = obj_blueTail10.oldPos;
	obj_blueTail12.pos = obj_blueTail11.oldPos;
	obj_blueTail13.pos = obj_blueTail12.oldPos;
	obj_blueTail14.pos = obj_blueTail13.oldPos;
	obj_blueTail15.pos = obj_blueTail14.oldPos;
	obj_blueTail16.pos = obj_blueTail15.oldPos;
	obj_blueTail17.pos = obj_blueTail16.oldPos;
	obj_blueTail18.pos = obj_blueTail17.oldPos;
	obj_blueTail19.pos = obj_blueTail18.oldPos;
	obj_blueTail20.pos = obj_blueTail19.oldPos;
	obj_blueTail21.pos = obj_blueTail20.oldPos;
	obj_blueTail22.pos = obj_blueTail21.oldPos;
	obj_blueTail23.pos = obj_blueTail22.oldPos;
	obj_blueTail24.pos = obj_blueTail23.oldPos;
	obj_blueTail25.pos = obj_blueTail24.oldPos;
	obj_blueTail26.pos = obj_blueTail25.oldPos;
	obj_blueTail27.pos = obj_blueTail26.oldPos;
	obj_blueTail28.pos = obj_blueTail27.oldPos;
	obj_blueTail29.pos = obj_blueTail28.oldPos;
	obj_blueTail30.pos = obj_blueTail29.oldPos;
	obj_blueTail31.pos = obj_blueTail30.oldPos;
	obj_blueTail32.pos = obj_blueTail31.oldPos;
}

// This Function consists of moving Player 2's snake according to their input - called in CheckGameState()
// Rotates head of snake according to direction of travel 
void MoveP2Snake() {
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	if (isPlaying) {
		switch (playerTwoInput) {
		case PlayerTwoInputType::UP:
			// Move head up
			obj_snakeHeadBlue.pos.y = obj_snakeHeadBlue.oldPos.y - cellHeight;
			MoveP2Tail();
			// Sets P2 Snake direction to be the current direction it is facing - so the player cannot travel from up to down, left to right and vice versa.
			snakeProperties.p2CurrentDirection = PLAY_PI;
			// Rotate head to face in the upward direction
			obj_snakeHeadBlue.rotation = snakeProperties.p2CurrentDirection;
			ResetTimer();
			break;
		case PlayerTwoInputType::LEFT:
			// Move head Left
			obj_snakeHeadBlue.pos.x = obj_snakeHeadBlue.oldPos.x - cellWidth;
			MoveP2Tail();
			snakeProperties.p2CurrentDirection = PLAY_PI / 2;
			// Rotate head to face in the left direction
			obj_snakeHeadBlue.rotation = snakeProperties.p2CurrentDirection;
			ResetTimer();
			break;
		case PlayerTwoInputType::DOWN:
			// Move head Down
			obj_snakeHeadBlue.pos.y = obj_snakeHeadBlue.oldPos.y + cellHeight;
			MoveP2Tail();
			snakeProperties.p2CurrentDirection = 0;
			// Rotate head to face in the downward direction
			obj_snakeHeadBlue.rotation = snakeProperties.p2CurrentDirection;
			ResetTimer();
			break;
		case PlayerTwoInputType::RIGHT:
			// Move head Right
			obj_snakeHeadBlue.pos.x = obj_snakeHeadBlue.pos.x + cellWidth;
			MoveP2Tail();
			snakeProperties.p2CurrentDirection = PLAY_PI * 1.5f;
			// Rotate head to face in the right direction
			obj_snakeHeadBlue.rotation = snakeProperties.p2CurrentDirection;
			ResetTimer();
			break;
		default:
			break;
		}
	}
}

// This Function checks which player (1 or 2) has achieved the predetermined max score first and therefore has won the game - called inside DrawGameObjects()
void CheckScore() {
	// The tailNumber starts off at 2 so I need to minus 2 from the current tail number in order to determine the current score (Both snakes have 2 tails at the beginning of the game by default)
	if (snakeProperties.p1TailNumber - 2 == maxScore) {
		p1Win = true;
		currentGameState = GameState::STATE_GAMEOVER;
	}
	else if (snakeProperties.p2TailNumber - 2 == maxScore) {
		p2Win = true;
		currentGameState = GameState::STATE_GAMEOVER;
	}
}

// This Function checks whether P1 wins, P2 wins or they both tie - called inside CheckBoundaryCollision() & CheckTailCollision()
void DetermineWinner() {
	// Check P1 Win OR P2 Win OR Both Win
	if (p1Win && p2Win) {
		// Both Snakes Died at the same time so it's a tie
		Play::PlayAudio("snake_tieGame");
	}
	else if (p1Win && p2Win == false) {
		// P1 Snake Wins & P2 Snake Loses
		Play::PlayAudio("snake_death");
		Play::PlayAudio("snake_p1Win");
	}
	else if (p2Win && p1Win == false) {
		// P2 Snake Wins & P1 Snake Loses
		Play::PlayAudio("snake_death");
		Play::PlayAudio("snake_p2Win");
	}
}

// This Function checks if either Player 1 and/or Player 2 have collided with their individual arena boundaries - called inside CheckGameState()
// Depending on the state of the Screen Wrapping checkbox the game will either constrain both the Player 1 snake and Player 2 snake to their own predetermined bounds 
// or will allow them to wrap to the opposite side of their arena 
void CheckBoundaryCollision() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);

	// Check if P1 snake is colliding with the red arena boundaries - if true then P2 Wins
	if (obj_snakeHeadRed.pos.x >= 700 && isScreenWrapEnabled == false || obj_snakeHeadRed.pos.x <= 98 && isScreenWrapEnabled == false || obj_snakeHeadRed.pos.y >= 681 && isScreenWrapEnabled == false || obj_snakeHeadRed.pos.y <= 119 && isScreenWrapEnabled == false) {
		p2Win = true;
		currentGameState = GameState::STATE_GAMEOVER;
	}
	else if (isScreenWrapEnabled == true) {
		int p1ArenaWidth = 700; 
		int p1ArenaHeight = 681;
		
		// Check P1 snake position relative to Width of Red arena
		// If colliding then wrap to other side
		if (obj_snakeHeadRed.pos.x >= p1ArenaWidth) {
			obj_snakeHeadRed.pos.x = 129;
		}
		else if (obj_snakeHeadRed.pos.x <= 98) {
			obj_snakeHeadRed.pos.x = 669;
		}	
		// Check P1 snake position relative to Height of Red arena
		// If colliding then wrap to other side
		if (obj_snakeHeadRed.pos.y >= p1ArenaHeight) {
			obj_snakeHeadRed.pos.y = 148;
		}	
		else if (obj_snakeHeadRed.pos.y <= 119) {
			obj_snakeHeadRed.pos.y = 652;
		} 
	}

	// Check if P2 snake is colliding with the blue arena boundaries - if true then P1 Wins
	if (obj_snakeHeadBlue.pos.x >= 1400 && isScreenWrapEnabled == false || obj_snakeHeadBlue.pos.x <= 798 && isScreenWrapEnabled == false || obj_snakeHeadBlue.pos.y >= 681 && isScreenWrapEnabled == false || obj_snakeHeadBlue.pos.y <= 119 && isScreenWrapEnabled == false) {
		p1Win = true;
		currentGameState = GameState::STATE_GAMEOVER;
	}
	else if (isScreenWrapEnabled == true) {
		int p2ArenaWidth = 1400;
		int p2ArenaHeight = 681;

		// Check P2 snake position relative to Width of Blue arena
		// If colliding then wrap to other side
		if (obj_snakeHeadBlue.pos.x >= p2ArenaWidth) {
			obj_snakeHeadBlue.pos.x = 830;
		}
		else if (obj_snakeHeadBlue.pos.x <= 798) {
			obj_snakeHeadBlue.pos.x = 1369;
		}
		// Check P2 snake position relative to Height of Blue arena
		// If colliding then wrap to other side
		if (obj_snakeHeadBlue.pos.y >= p2ArenaHeight) {
			obj_snakeHeadBlue.pos.y = 148;
		}
		else if (obj_snakeHeadBlue.pos.y <= 119) {
			obj_snakeHeadBlue.pos.y = 652;
		}
	}
	DetermineWinner();
}

// This Function checks to see if either Player 1 and/or Player 2 have collided with their own tail - called inside CheckGameState()
// Challenges both Player 1 and Player 2 to think carefully about their choice of movement as the game progresses
void CheckTailCollision() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	std::vector<int> v_p1Tails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILRED);
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	std::vector<int> v_p2Tails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILBLUE);

	// Retrieve all current P1 tail segments from the vector data structure 
	for (int id : v_p1Tails) {
		GameObject& obj_redTails = Play::GetGameObject(id);
		if (Play::IsColliding(obj_snakeHeadRed, obj_redTails)) {
			p2Win = true;
			currentGameState = GameState::STATE_GAMEOVER;
		}
	}
	// Retrieve all current P2 tail segments from the vector data structure 
	for (int id : v_p2Tails) {
		GameObject& obj_blueTails = Play::GetGameObject(id);
		if (Play::IsColliding(obj_snakeHeadBlue, obj_blueTails)) {
			p1Win = true;
			currentGameState = GameState::STATE_GAMEOVER;
		}
	}
	DetermineWinner();
}

// This Function checks if the P1 snakes head has collided with any of its tail segments - called inside CheckGameState()
void P1TailCollision() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	std::vector<int> v_p1Tails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILRED);

	// Retrieve all current P1 tail segments from the vector data structure 
	for (int id : v_p1Tails) {
		GameObject& obj_redTails = Play::GetGameObject(id);
		if (Play::IsColliding(obj_snakeHeadRed, obj_redTails)) {
			// Kill P1 Snake -> P2 Wins
			Play::PlayAudio("snake_death");
			Play::PlayAudio("snake_p2Win");
			p2Win = true;
			currentGameState = GameState::STATE_GAMEOVER;
		}
	}
}

// This Function checks if the P2 snakes head has collided with any of its tail segments - called in CheckGameState()
void P2TailCollision() {
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	std::vector<int> v_p2Tails = Play::CollectGameObjectIDsByType(MainGame::GameObjectType::TYPE_SNAKETAILBLUE);

	// Retrieve all current P2 tail segments from the vector data structure 
	for (int id : v_p2Tails) {
		GameObject& obj_blueTails = Play::GetGameObject(id);
		if (Play::IsColliding(obj_snakeHeadBlue, obj_blueTails)) {
			// Kill P2 Snake -> P1 Wins
			Play::PlayAudio("snake_death");
			Play::PlayAudio("snake_p1Win");
			p1Win = true;
			currentGameState = GameState::STATE_GAMEOVER;
		}
	}
}

// This Function consists of randomizing the spawning of the Food for P1 snake - called inside MainGameUpdate()
void RedRandFoodSpawn() {
	// Choose a random number between 0 and 10 (the index for the range of elements in both the 'redCenterCellsX' & redCenterCellsY' array)
	redRandX = redCenterCellsX[rand() % 10];
	redRandY = redCenterCellsY[rand() % 10];
}

// This Function consists of randomizing the spawning of the Food for P2 snake- called inside MainGameUpdate()
void BlueRandFoodSpawn() {
	// Choose a random number between 0 and 10 (the index for the range of elements in both the 'blueCenterCellsX' & blueCenterCellsY' array)
	blueRandX = blueCenterCellsX[rand() % 10];
	blueRandY = blueCenterCellsY[rand() % 10];
}

// This Function consists of spawning the item in a random location within a range inside P1's arena once the P1 snake has eaten it - called inside EatItem()
void SpawnItemP1(){
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	GameObject& obj_itemRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_ITEMRED);

	// Check if the food's predetermined co-ordinates are not in the same position as the red snakes head segment
	// If true, then it will spawn the item at its random location
	// If false, then it will re-roll the randomization and check again
	// Food can spawn in on the tail as it adds challenge to the game - requiring patience and good-decision making skills, especially in late-game
	if (Point2f{ redRandX, redRandY } != obj_snakeHeadRed.pos) {
		obj_itemRed.pos.x = redRandX;
		obj_itemRed.pos.y = redRandY;
	}
	else if (Point2f{ redRandX, redRandY } == obj_snakeHeadRed.pos)  {
		// Re-Randomise and spawn food
		RedRandFoodSpawn();
		// Spawn food in a new grid cell location not occupied by the red snake head
		obj_itemRed.pos.x = redRandX;
		obj_itemRed.pos.y = redRandY;
	}
}

// This Function consists of spawning the item in a random location within a range inside P2's arena once the P2 snake has eaten it - called inside EatItem()
void SpawnItemP2() {
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	GameObject& obj_itemBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_ITEMBLUE);

	// Check if the food's predetermined co-ordinates are not in the same position as the blue snakes head segment
	// If true, then it will spawn the item at its random location
	// If false, then it will re-roll the randomization and check again
	// Food can spawn in on the tail as it adds challenge to the game - requiring patience and good-decision making skills, especially in late-game
	if (Point2f{ blueRandX, blueRandY } != obj_snakeHeadBlue.pos) {
		obj_itemBlue.pos.x = blueRandX;
		obj_itemBlue.pos.y = blueRandY;
	}
	else if (Point2f{ blueRandX, blueRandY } == obj_snakeHeadBlue.pos) {
		// Re-Randomise and spawn food
		BlueRandFoodSpawn();
		// Spawn food in a new grid cell location not occupied by the blue snake head
		obj_itemBlue.pos.x = blueRandX;
		obj_itemBlue.pos.y = blueRandY;
	}
}

// This Function instantiates a new 'Red Tail' GameObject -> grows player 1's snake tail by 1 - called inside EatItem()
void InstantiateRedTail() { 
	// Increment Player 1's total tail segments by 1 
	snakeProperties.p1TailNumber += 1;
	// Create new Tail - set initial position off screen
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKETAILRED, { -100 , -100 }, 25, "snakeTailRed");
}

// This Function instantiates a new 'Blue Tail' GameObject -> grows player 2's snake tail by 1 - called inside EatItem()
void InstantiateBlueTail() { 
	// Increment Player 2's total tail segments by 1 
	snakeProperties.p2TailNumber += 1;
	// Create new Tail - set initial position off screen
	Play::CreateGameObject(MainGame::GameObjectType::TYPE_SNAKETAILBLUE, { -100 , -100 }, 25, "snakeTailBlue");	
}

// This Function checks if the P1 and/or P2 snake is colliding with an item and if so, calls multiple functions - called inside CheckGameState()
void EatItem() {
	GameObject& obj_snakeHeadRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADRED);
	GameObject& obj_snakeHeadBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_SNAKEHEADBLUE);
	GameObject& obj_itemRed = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_ITEMRED);
	GameObject& obj_itemBlue = Play::GetGameObjectByType(MainGame::GameObjectType::TYPE_ITEMBLUE);

	// Checks if both P1 and/or P2 are colliding with their specified food
	if (Play::IsColliding(obj_snakeHeadRed, obj_itemRed)) { 
		SpawnItemP1();
		InstantiateRedTail();
		Play::PlayAudio("snake_eat");
	}
	if (Play::IsColliding(obj_snakeHeadBlue, obj_itemBlue)) {
		SpawnItemP2();
		InstantiateBlueTail();
		Play::PlayAudio("snake_eat");
	}
}