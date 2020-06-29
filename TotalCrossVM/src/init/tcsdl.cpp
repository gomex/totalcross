// Copyright (C) 2000-2013 SuperWaba Ltda.
// Copyright (C) 2014-2020 TotalCross Global Mobile Platform Ltda.
//
// SPDX-License-Identifier: LGPL-2.1-only

#define DISPLAY_INDEX 0
#define NO_FLAGS 0
#define IS_NULL(x)      ((x) == NULL)
#define NOT_SUCCESS(x)  ((x) != 0)
#define SUCCESS(x)      ((x) == 0)

#include "tcsdl.h"
#include <iostream>
#include <vector>

static SDL_Renderer* renderer;
static SDL_Texture* texture;

/*
 * Init steps to create a window and texture to Skia handling
 *
 * Args:
 * - ScreenSurface from TotalCross globals
 *
 * Return:
 * - false on failure
 * - true on success
 */
bool TCSDL_Init(ScreenSurface screen, const char* title, bool fullScreen) {
SDL_Init( 0 );

	std::cout << "Testing video drivers..." << '\n';
	std::vector< bool > drivers(SDL_GetNumVideoDrivers());

	for (int i = 0; i < drivers.size(); ++i) {
		drivers[i] = (0 == SDL_VideoInit(SDL_GetVideoDriver(i)));
		SDL_VideoQuit();
	}

	std::cout << "SDL_VIDEODRIVER available:";
	for (int i = 0; i < drivers.size(); ++i) {
		std::cout << " " << SDL_GetVideoDriver(i);
	}
	std::cout << '\n';

	std::cout << "SDL_VIDEODRIVER usable   :";
	for (int i = 0; i < drivers.size(); ++i) {
      if( !drivers[ i ] ) continue;
		std::cout << " " << SDL_GetVideoDriver(i);
	}
	std::cout << '\n';

	// Only init video (without audio)
	if (NOT_SUCCESS(SDL_Init(SDL_INIT_VIDEO))) {
		std::cerr << "SDL_Init(): " << SDL_GetError() << '\n';
		return false;
	}
	std::cout << "SDL_VIDEODRIVER selected : " << SDL_GetCurrentVideoDriver() << '\n';

	SDL_DisplayMode DM;
	// Get current display mode of all displays.
  	for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i){
		int should_be_zero = SDL_GetCurrentDisplayMode(i, &DM);

		if(should_be_zero != 0) {
			std::cerr << "SDL_GetCurrentDisplayMode() failed for video display #" << i << ": " << SDL_GetError() << '\n';
		} else {
			std::cout << "SDL_DisplayMode #" << i << ": current display mode is " << DM.w << "x" << DM.h << "x" << DM.refresh_rate << '\n';
		}
	}

	int width = (getenv("TC_WIDTH")  == NULL) ? DM.w : std::stoi(getenv("TC_WIDTH"));
	int height = (getenv("TC_HEIGHT") == NULL) ? DM.h : std::stoi(getenv("TC_HEIGHT"));

	uint32 flags; 
	if(getenv("TC_FULLSCREEN") == NULL) {
		flags = SDL_WINDOW_FULLSCREEN;
	} else {
		flags = SDL_WINDOW_SHOWN;
		width -= width*0.09;
		height -= height*0.09;
	}

	// Create the window
	SDL_Window* window;
	if (IS_NULL(window = SDL_CreateWindow(
							 title,
							 SDL_WINDOWPOS_UNDEFINED,
							 SDL_WINDOWPOS_UNDEFINED,
							 640,
							 400,
							 SDL_WINDOW_SHOWN
						 ))) {
		std::cerr << "SDL_CreateWindow(): " << SDL_GetError() << '\n';
		return false;
	}

	std::cout << "SDL_RENDER_DRIVER available:";
	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		std::cout << " " << info.name;
	}
	std::cout << '\n';

	// Create a 2D rendering context for a window
	if (IS_NULL(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))) {
		std::cerr << "SDL_CreateRenderer(): " << SDL_GetError() << '\n';
		std::cout << '\n' << "HINT: try to export SDL_RENDER_DRIVER environment variable with an available render driver!" << '\n';
		return false;
	}

	// Get renderer driver information
	SDL_RendererInfo rendererInfo;
	if (NOT_SUCCESS(SDL_GetRendererInfo(renderer, &rendererInfo))) {
		std::cerr << "SDL_GetRendererInfo(): " << SDL_GetError() << '\n';
		return 0;
	}
	std::cout << "SDL_RENDER_DRIVER selected : " << rendererInfo.name << '\n';
//     bool running = true;
//     unsigned char i = 0;
//     while( running )
//     {
//         SDL_SetRenderDrawColor( renderer, i, i, i, SDL_ALPHA_OPAQUE );
//         SDL_RenderClear( renderer );
//         SDL_RenderPresent( renderer );
//         i+=5;
//     }

//     SDL_DestroyRenderer( renderer );
//     SDL_DestroyWindow( window );
//     SDL_Quit();


//   // Only init video (without audio)
//   if(NOT_SUCCESS(SDL_Init(SDL_INIT_VIDEO))) {
//     printf("SDL_Init failed: %s\n", SDL_GetError());
//     return false;
//   }

//   // Get the desktop area represented by a display, with the primary
//   // display located at 0,0 based on viewport allocated on initial position
//   int (*TCSDL_GetDisplayBounds)(int, SDL_Rect*) = 
// #ifdef __arm__                  
//     &SDL_GetDisplayBounds;
// #else                           
//     &SDL_GetDisplayUsableBounds;
// #endif

//   SDL_Rect viewport;
//   if(NOT_SUCCESS(TCSDL_GetDisplayBounds(DISPLAY_INDEX, &viewport))) {
//     printf("SDL_GetDisplayBounds failed: %s\n", SDL_GetError());
//     return false;
//   }

//   // Adjust height on desktop, it should not affect fullscreen (y should be 0)
//   viewport.h -= viewport.y;

//   // Create the window
//   if(IS_NULL(window = SDL_CreateWindow(
//                                 title, 
//                                 viewport.x,
//                                 viewport.y, 
//                                 viewport.w, 
//                                 viewport.h, 
//                                 (fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_MAXIMIZED)
//                                 ))) {
//     printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
//     return false;
//   }

//   // Get the size of the window's client area
//   SDL_GetWindowSize(window, &viewport.w, &viewport.h);

//   // Create a 2D rendering context for a window
//   if(IS_NULL(renderer = SDL_CreateRenderer(window, -1, NO_FLAGS))) {
//     printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
//     return false;
//   }

//   // Get renderer driver information
//   SDL_RendererInfo rendererInfo;
//   if (NOT_SUCCESS(SDL_GetRendererInfo(renderer, &rendererInfo))) {
//     printf("SDL_GetRendererInfo failed: %s\n", SDL_GetError());
//     return 0;
//   } else {
//     // Set render driver 
//     if ((SDL_SetHint(SDL_HINT_RENDER_DRIVER, rendererInfo.name)) == SDL_FALSE) {
//       printf("SDL_SetHint failed: %s\n", SDL_GetError());
//       return false;
//     }
//   }

//   // Set renderer dimensions
//   if (NOT_SUCCESS(SDL_GetRendererOutputSize(
//                                 renderer, 
//                                 &viewport.w, 
//                                 &viewport.h))) {
//     printf("SDL_GetRendererOutputSize failed: %s\n", SDL_GetError());
//     return false;
//   }
  
	// Get window pixel format
	Uint32 windowPixelFormat;
	if (SDL_PIXELFORMAT_UNKNOWN == (windowPixelFormat = SDL_GetWindowPixelFormat(window))) {
		std::cerr << "SDL_GetWindowPixelFormat(): " << SDL_GetError() << '\n';
		return false;
	}

	// MUST USE SDL_TEXTUREACCESS_STREAMING, CANNOT BE REPLACED WITH SDL_CreateTextureFromSurface
	if (IS_NULL(texture = SDL_CreateTexture(
							  renderer,
							  windowPixelFormat,
							  SDL_TEXTUREACCESS_STREAMING,
							  640,
							  400))) {
		std::cerr << "SDL_CreateTexturet(): " << SDL_GetError() << '\n';
		return false;
	}
	// Get pixel format struct
	SDL_PixelFormat* pixelformat;
	if (IS_NULL(pixelformat = SDL_AllocFormat(windowPixelFormat))) {
		std::cerr << "SDL_AllocFormat(): " << SDL_GetError() << '\n';
		return false;
	}

	// Adjusts screen width to the viewport
	screen->screenW = 640;
	// Adjusts screen height to the viewport
	screen->screenH = 400;
	// Adjusts screen's BPP
	screen->bpp = pixelformat->BitsPerPixel;
	// Set surface pitch
	screen->pitch = pixelformat->BytesPerPixel * screen->screenW;
	// pixel order
	screen->pixelformat = windowPixelFormat;
	// Adjusts screen's pixel surface
	if (IS_NULL(screen->pixels = (uint8*) malloc(screen->pitch * screen->screenH))) {
		std::cerr << "Failed to alloc " << (screen->pitch * screen->screenH) << " bytes for pixel surface" << '\n';
		return false;
	}

	if (IS_NULL(screen->extension = (ScreenSurfaceEx) malloc(sizeof(TScreenSurfaceEx)))) {
		free(screen->pixels);
		std::cerr << "Failed to alloc TScreenSurfaceEx of " << sizeof(TScreenSurfaceEx) << " bytes" << '\n';
		return false;
	}
	SCREEN_EX(screen)->window = window;
	SCREEN_EX(screen)->renderer = renderer;
	SCREEN_EX(screen)->texture = texture;

	SDL_FreeFormat(pixelformat);

    // bool running = true;
    // unsigned char i = 0;

    // printf("teste kappa 2\n");
    // while( running )
    // {


    //     for (int y = 0; y < screen->screenH; ++y)
    //     {
    //         for (int x = 0; x < screen->screenW; ++x)
    //         {
    //             screen->pixels[x + y * screen->screenW] = 0xffffff;
    //         }
    //     }

    //     // SDL_SetRenderDrawColor( renderer, i, i, i, SDL_ALPHA_OPAQUE );
        
    //     SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    //     SDL_RenderCopy(renderer, texture, NULL, NULL);
    //     SDL_RenderPresent( renderer );
    //     SDL_RenderClear( renderer );
    //     i+=5;
    // }

	return true;
}

/*
 * Update the given texture rectangle with new pixel data.
 *
 * # MUST BE REVIEWED
 *
 * SDL_UpdateTexture it's too slow and our implementation
 * depends on it
 */
void TCSDL_UpdateTexture(int w, int h, int pitch, void* pixels) {
	// Update the given texture rectangle with new pixel data.
	SDL_UpdateTexture(texture, NULL, pixels, pitch);
	// Call SDL render present
	TCSDL_Present();
}

/*
 * Update the screen with rendering performed
 */
void TCSDL_Present() {
	// Copy a portion of the texture to the current rendering target
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	// Update the screen with rendering performed
	SDL_RenderPresent(renderer);
	// Clears the entire rendering targe
	SDL_RenderClear(renderer);
}

/*
 * Destroy all SDL allocated variables
 */
void TCSDL_Destroy(ScreenSurface screen) {
	if (screen->pixels != NULL) {
		free(screen->pixels);
	}

	if (SCREEN_EX(screen) != NULL) {
		SDL_DestroyTexture(SCREEN_EX(screen)->texture);
		SDL_DestroyRenderer(SCREEN_EX(screen)->renderer);
		SDL_DestroyWindow(SCREEN_EX(screen)->window);
		free(SCREEN_EX(screen));
	}
	SDL_Quit();
}

void TCSDL_GetWindowSize(ScreenSurface screen, int32* width, int32* height) {
	SDL_GetWindowSize(SCREEN_EX(screen)->window, width, height);
}
