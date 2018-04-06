#ifndef draw_h
#define draw_h

#include "window.h"

// color used when clearing the screen
#define CLEAR_COLOR /*red:*/ 0, /*blue:*/ 0, /*green:*/ 0, /*alpha:*/ 1.0


/* START LOOP
 * ----------
 * Initialise and launch main loop, begining drawing and event tracking operations
 *
 * @param: sdl window (see window.h)
 *
 * Here lies the heart of the program, startLoop won't return until a quit event is fired
 */
void startLoop(SDL_Window *win);

#endif /* draw_h */