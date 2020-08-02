#include <hal/debug.h>
#include <hal/xbox.h>
#include <hal/video.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>

static void printSDLErrorAndReboot(void)
{
    debugPrint("SDL_Error: %s\n", SDL_GetError());
    debugPrint("Rebooting in 5 seconds.\n");
    Sleep(5000);
    XReboot();
}

static void printIMGErrorAndReboot(void)
{
    debugPrint("SDL_Image Error: %s\n", IMG_GetError());
    debugPrint("Rebooting in 5 seconds.\n");
    Sleep(5000);
    XReboot();
}


struct vector2{
	int x;
	int y;
};

vector2 getCoords(vector2 pos) {
	vector2 ret;
	ret.x = (120+(pos.x*120))+5;
	ret.y = (pos.y*120)+5;
	return ret;
}

void game(void)
{
	SDL_Surface* imgs[12] = {};
    int done = 0;
    SDL_Window *window;
    SDL_Event event;
    SDL_Surface *screenSurface;
    SDL_GameController* gameController = NULL;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL video.\n");
        printSDLErrorAndReboot();
    }

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");

    if (SDL_NumJoysticks() < 1) {
    	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find any joysticks.\n");
    	printSDLErrorAndReboot();
    }
    else {
    	if (SDL_IsGameController(0)) {
    		gameController = SDL_GameControllerOpen(0);
    	}
    	if (gameController == NULL) {
    		printSDLErrorAndReboot();
    	}
    }

    window = SDL_CreateWindow("2048X",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        debugPrint( "Window could not be created!\n");
        SDL_VideoQuit();
        printSDLErrorAndReboot();
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't intialize SDL_image.\n");
        SDL_VideoQuit();
        printIMGErrorAndReboot();
    }

    screenSurface = SDL_GetWindowSurface(window);
    if (!screenSurface) {
        SDL_VideoQuit();
        printSDLErrorAndReboot();
    }
    // Load all images
    imgs[0] = IMG_Load("D:\\pureblack.png");
    for (int i = 1; i < 12; i++) {
    	std::string imgPath = "D:\\"+std::to_string(1<<i)+".png";
    	SDL_Surface* surface = IMG_Load(imgPath.c_str());
    	if (!surface) {
    		SDL_VideoQuit();
        	printSDLErrorAndReboot();
    	}
    	imgs[i] = surface;
    }
    // Setup test pattern
    SDL_Surface* tilearray[4][4] = { nullptr };
    for (int x = 0; x < 4; x++) {
    	for (int y = 0; y < 4; y++) {
    		tilearray[x][y] = imgs[(x+(y*4))%12];
    	}
    }
    int i = 0;
    while (!done) {
        XVideoWaitForVBlank();
        tilearray[i%4][i/4] = imgs[0];
        /* Check for events */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_CONTROLLERBUTTONDOWN:
        		switch (event.cbutton.button) {
        			case SDL_CONTROLLER_BUTTON_DPAD_UP:
        				tilearray[0][0] = imgs[11];
        				break;
        			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        				tilearray[1][0] = imgs[11];
        				break;
        			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        				tilearray[2][0] = imgs[11];
        				break;
        			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        				tilearray[3][0] = imgs[11];
        				break;
        			default:
        				break;
            	}
                break;
            default:
                break;
            }
        }
        for (int x = 0; x < 4; x++) {
        	for (int y = 0; y < 4; y++) {
        		if (tilearray[x][y] != nullptr) {
        			vector2 pos;
        			pos.x = x;
        			pos.y = y;
        			vector2 topleft = getCoords(pos);
        			SDL_Rect dst = {topleft.x,topleft.y,110,110};
        			SDL_BlitSurface(tilearray[x][y], NULL, screenSurface, &dst);
        		}
    		}
    	}
        SDL_UpdateWindowSurface(window);
        i++;
    }

    IMG_Quit();
    SDL_Quit();
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    game();
    return 0;
}
