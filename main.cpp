#include <stdio.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <utility>
#include <vector>
#include <algorithm>
#if defined(NXDK)
#include <hal/debug.h>
#include <hal/xbox.h>
#include <hal/video.h>
#include <windows.h>
#define PATH_SEP "\\"
#define DATA_PATH "D:" PATH_SEP
#else
#define debugPrint(...) printf(__VA_ARGS__)
#define XVideoSetMode(...)
#define Sleep(x) SDL_Delay(x)
#define XReboot(...) exit(555)
#define XVideoWaitForVBlank(...) SDL_Delay(1000/60-1)
#define PATH_SEP "/"
#define DATA_PATH "." PATH_SEP
#endif

#define SQUARE_SIZE 5

struct collisionRet {
    bool collided = false;
    std::vector<std::vector<int>> retGrid;
    bool actuallydone = false;
};
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

void printMultidimensionalVector(std::vector<std::vector<int>> vec, std::string message = "") {
#ifdef _DEBUG
    printf(message.c_str());
    printf("\nSTART OF VEC:\n");
    for (auto row : vec) {
        for (auto col : row) {
            printf("%u ",col);
        }
        printf("\n");
    }
    printf("END OF VEC;\n");
#endif
}

enum direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class vector2{
public:
    int x;
    int y;
    vector2() {
        x = 0;
        y = 0;
    }
    vector2(int inx, int iny) {
        x = inx;
        y = iny;
    }
    vector2 operator+(vector2 const& other) {
        return vector2{x+other.x,y+other.y};
    }
    vector2& operator+=(vector2 const& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    vector2& operator-=(vector2 const& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    vector2 operator-(vector2 const& other) {
        return vector2{x-other.x,y-other.y};
    }
    bool operator==(vector2 const& other) {
        return (x == other.x && y == other.y);
    }
};

vector2 offsets[4] = {{0,-1},{0,1},{-1,0},{1,0}};

struct tileRet {
    std::vector<std::vector<int>> ret;
    bool done;
};

tileRet addTile(std::vector<std::vector<int>> tilearray) {
    bool done = false;
    int count = 0;
    for (int x = 0; x < SQUARE_SIZE; x++) {
        for (int y = 0; y < SQUARE_SIZE; y++) {
            if (tilearray[x][y] == 0) {
                count++;
            }
        }
    }
    if (count == 0) {
        done = true;
    }
    int rand1DPos = rand()%(count+1);
    vector2 randPos = {rand1DPos%SQUARE_SIZE,rand1DPos/SQUARE_SIZE};
    while (tilearray[randPos.x][randPos.y] != 0) {
        rand1DPos++;
        randPos = {rand1DPos%SQUARE_SIZE,rand1DPos/SQUARE_SIZE};
    }
    tilearray[randPos.x][randPos.y] = 1;
    return {tilearray,done};
}
vector2 getCoords(vector2 pos) {
	vector2 ret;
	int tileSize = 480 / SQUARE_SIZE;
	ret.x = (pos.x * tileSize);
	ret.y = (pos.y * tileSize);
	return ret;
}
std::string tileMove[4] = {"UP","DOWN","LEFT","RIGHT"};
collisionRet handleMovement(std::vector<std::vector<int>> gameGrid, direction offset, SDL_Surface* imgs[12]) {
    bool collided = false;
    std::vector<std::vector<int>> scanGrid = gameGrid;
    vector2 offset_vec = offsets[(int) offset];
    printMultidimensionalVector(gameGrid,"Start of function\nMoving tiles "+tileMove[(int)offset]+" at offset "+std::to_string((int) offset));
    for (int x = 0; x < SQUARE_SIZE; x++) {
        for (int y = 0; y < SQUARE_SIZE; y++) {
            if (scanGrid[x][y] != 0) {
                vector2 temp_vec{x,y};
                bool local_collision = false;
                temp_vec+=offset_vec;
                while (!(temp_vec.x < 0 || temp_vec.x > SQUARE_SIZE-1 || temp_vec.y < 0 || temp_vec.y > SQUARE_SIZE-1)) {
                    if (scanGrid[temp_vec.x][temp_vec.y] == scanGrid[x][y]) {
                        collided = true;
                        local_collision = true;
                        break;
                    }
                    temp_vec+=offset_vec;
                }
                int offset = 0;
                if (local_collision) {
                    offset = 1;
                }
                else {
                    temp_vec -= offset_vec;
                }
                gameGrid[temp_vec.x][temp_vec.y] = gameGrid[x][y] + offset;
                gameGrid[x][y] = 0;
                printMultidimensionalVector(gameGrid,"After algorithm for one tile");
            }
        }
    }
    printMultidimensionalVector(gameGrid,"After entire algorithm");
    return {collided,gameGrid,true};
}

void game(void)
{
    srand((unsigned int)time(NULL));
	SDL_Surface* imgs[12] = {};
    int done = 0;
    SDL_Window *window;
    SDL_Event event;
    SDL_Surface *screenSurface;
    SDL_GameController* gameController = NULL;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
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
    imgs[0] = IMG_Load(DATA_PATH"pureblack.png");
    for (int i = 1; i < 12; i++) {
    	std::string imgPath = DATA_PATH+std::to_string(1<<i)+".png";
    	SDL_Surface* surface = IMG_Load(imgPath.c_str());
    	if (!surface) {
    		SDL_VideoQuit();
        	printSDLErrorAndReboot();
    	}
    	imgs[i] = surface;
    }
    // Setup test pattern
    std::vector<std::vector<int>> tilearray;
    for (int i = 0; i < SQUARE_SIZE; i++) {
        tilearray.push_back(std::vector<int>());
    }
    for (int i = 0; i < SQUARE_SIZE; i++) {
        for (int j = 0; j < SQUARE_SIZE; j++) {
            tilearray[i].push_back(0);
        }
    }
    for (int i = 0; i < 2; i++) {
        tilearray = addTile(tilearray).ret;
    }
    int i = 0;
    while (!done) {
        XVideoWaitForVBlank();
        /* Check for events */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = 1;
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                    collisionRet ret = {false,tilearray,false};
                    switch (event.cbutton.button) {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            ret = handleMovement(tilearray, UP, imgs);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            ret = handleMovement(tilearray, DOWN, imgs);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                            ret = handleMovement(tilearray, LEFT, imgs);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                            ret = handleMovement(tilearray, RIGHT, imgs);
                            break;
                        default:
                            break;
                    }
                    tilearray = ret.retGrid;
                    if (!ret.collided && ret.actuallydone) {
                        tilearray = addTile(tilearray).ret;
                    }
                    break;
            }
        }
        for (int x = 0; x < SQUARE_SIZE; x++) {
        	for (int y = 0; y < SQUARE_SIZE; y++) {
        		vector2 pos;
        		pos.x = x;
        		pos.y = y;
        		vector2 topleft = getCoords(pos);
        		SDL_Rect dst = {topleft.x,topleft.y,110,110};
        		SDL_BlitScaled(imgs[tilearray[x][y]], NULL, screenSurface, &dst);
    		}
    	}
        SDL_UpdateWindowSurface(window);
        i++;
    }

    IMG_Quit();
    SDL_Quit();
    exit(1);
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    game();
    return 0;
}
