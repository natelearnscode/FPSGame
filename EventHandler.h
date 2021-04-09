#include <SDL.h>
#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

//An event handler to handle SDL Events such as keyboard and mouse inputs
class EventHandler {
public:
	bool moveForward;
	bool moveBackward;
	bool moveLeft;
	bool moveRight;
	int mouseXPos;
	int mouseYPos;

	EventHandler() : moveForward(false), moveBackward(false), moveLeft(false), moveRight(false) {};

	void handleEvent(SDL_Event e) {
		//Check key down events to set flags for if the character should move in a direction
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w) { //If "w" is pressed
			moveForward = true;
		}
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s) { //If "s" is pressed
			moveBackward = true;
		}
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a) { //If "a" is pressed
			moveLeft = true;
		}
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d) { //If "d" is pressed
			moveRight = true;
		}

		//Check key up events to set flags for if the character should stop moving in a direction
		if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_w) { //If "w" is released
			moveForward = false;
		}
		if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s) { //If "s" is released
			moveBackward = false;
		}
		if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a) { //If "a" is released
			moveLeft = false;
		}
		if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d) { //If "d" is released
			moveRight = false;
		}

		//Check mouse event
		if (e.type == SDL_MOUSEMOTION) {
			SDL_GetMouseState(&mouseXPos,&mouseYPos);
		}
	};
};
#endif