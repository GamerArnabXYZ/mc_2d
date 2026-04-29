#pragma once

#include <SDL2/SDL.h>

#if __has_include(<SDL2/SDL_image.h>)
  #include <SDL2/SDL_image.h>
#elif __has_include(<SDL_image.h>)
  #include <SDL_image.h>
#else
  #include "SDL_image.h"
#endif

#if __has_include(<SDL2/SDL_ttf.h>)
  #include <SDL2/SDL_ttf.h>
#elif __has_include(<SDL_ttf.h>)
  #include <SDL_ttf.h>
#else
  #include "SDL_ttf.h"
#endif

#if __has_include(<SDL2/SDL_mixer.h>)
  #include <SDL2/SDL_mixer.h>
#elif __has_include(<SDL_mixer.h>)
  #include <SDL_mixer.h>
#endif
