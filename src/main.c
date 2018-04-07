#include "../include/window.h"
#include "../include/draw.h"
#include "../include/messages.h"
#include "../include/load.h"

#define WINDOW_TITLE "Flap-imac"

World obstacleMap;

int main(int argc, char** argv) {
    SDL_Window *window = NULL;
    
    
    // initialise SDL and show a new window on screen
    if (!initWindow(&window, WINDOW_TITLE)) return 0;
    
    loadWorld("/Users/piergabory/Developer/IMAC/Computer Graphics/Projet/projet_rasterisation/image.ppm", &obstacleMap, &enemies, &bonuses);
    
    
    // start main loop, updating the screen at a set refresh rate
    // - see draw(), static function in draw.h
    startLoop(window);
    
    // free SDL ressources once the loop ended
    SDL_Quit();
    
    return 1;
}
