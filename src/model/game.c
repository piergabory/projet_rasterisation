#include "model/game.h"

//// GETTERS

/**
 * Level Getter
 * @return Level of the active game structure
 */
Level getLevel() {return *(gm->level); }


/**
 * Player mob Getter
 * @return Mob of the player in the active game structure
 */
Mob getPlayer() { return *(gm->player); }


/**
 * Mob List Getter
 * @param char Mobtype (defined in model/structs/mob.struct.h)
 * @return MobList of selected type, from the active game structure
 */
MobList getMobList(char mob) {
    switch (mob) {
        case 'e': return gm->enemies;
        case 'b': return gm->bonuses;
        case 'j': return gm->projectiles;
        case 't': return gm->ennemyProjectiles;
        case 'p': return gm->player;
        default: return NULL;
    }
}




//// GAME METHODS

/**
 * Initialise Game
 * ---------------
 * Set the game structure to its sarting state
 * @return int success status boolean
 */
int initGame() {
    // set random seed
    srand(time(NULL));
    
    // allocate game structure
    gm = allocGame();

    
    // initialize mob lists
    gm->enemies = NULL;
    gm->bonuses = NULL;
    gm->projectiles = NULL;
    gm->ennemyProjectiles = NULL;
    
    
    // set level, ennemies and bonuses data
    if(loadWorld("map.ppm", gm) != 0){
        printf(ERR_LOADWORLD);
        return 0;
    }
    
    
    // initialize level status to 'playing'
    gm->level->status = STATUS_PLAYING;
    
    
    // -- initialise player
    
    // set player default position
    gm->player->px = 0.01;
    gm->player->py = 0.5;
    
    // velocity vector
    gm->player->vx = 0;
    gm->player->vy = 0;
    
    gm->player->health = 20;
    gm->player->type = PLAYER;
    gm->player->next = NULL;
    
    return 1;
}


/**
 * Reset Game
 * ----------
 * 1. De-allocate active game structure off memory
 * 2. initialise a new one.
 */
void resetGame() {
    freeGame(gm);
    initGame();
}


/**
 * Update Game
 * -----------
 * Controls the frame's computations for
 *    - Collisions
 *    - Physics
 *    - Player/Game Status/Progress/health
 *
 * Should be called in the SDL main loop
 */
void updateGame() {
    float level_screen_size = (getViewportWidth()/(float)getViewportHeight()) * gm->level->height;
    float zone_size = level_screen_size/(float)gm->level->width;
    
    // stop scrolling at the end of the map
    if (gm->level->progress < 1 - zone_size)
        gm->level->progress += PROGRESS_RATE;

    // update player
    updatePlayer(gm->player);
    
    //// PLAYER COLLISIONS
    
    // Terrain
    int terrainType = isMobOnTerrain(*(gm->player), *(gm->level));
    if(terrainType != 0){
        printf("ouch!\n");
        gm->player->vx *= -.5;
        if (gm->player->vx >= 0)
            gm->player->vx -= 2*PROGRESS_RATE;
        
        gm->player->vy *= -.5;
        if(terrainType == 18)
            gm->level->status = 1;
        else
            playerHealth(WALL_DAMAGE);
    }
    
    // player cant leave the screen.
    if (gm->player->px < gm->level->progress) gm->player->px = gm->level->progress;
    
    // Bonus, Enemy and Enemy Projectile
    playerHit(&(gm->bonuses), BONUS_HEALTH);
    playerHit(&(gm->enemies), ENEMY_DAMAGE);
    playerHit(&(gm->ennemyProjectiles), PROJECTILE_DAMAGE);
    
    //// END PLAYER COLLISIONS

    
    //// UPDATE MOBS
    MobList *curr = NULL;
    MobList *target = NULL;
    
    
    for (curr = &(gm->enemies); *curr != NULL; curr = &((*curr)->next)) {
        if(isHidden(**curr)) continue;
        
        updateEnnemy(*curr, *(gm->player));
        
        // ennemy shoot randomly if align with the player and is visible
        if ((rand() % 300) == 0 && fabs((*curr)->py - gm->player->py) < 0.1) {
            ennemyShoot(**curr);
        }
    }
    
    for (curr = &(gm->ennemyProjectiles); *curr != NULL; curr = &((*curr)->next)) {
        if(isHidden(**curr)) continue;
        updateProjectile(*curr);
    }
    
    // projectiles
    for (curr = &(gm->projectiles); *curr != NULL; curr = &((*curr)->next)) {
        // projectile is flying
        updateProjectile(*curr);
        
        // projectile hit wall
        if (isMobOnTerrain(**curr, *(gm->level)) || isHidden(**curr)) {
            freeMob(curr);
            break;
        }
            
        // projectile hit enemy
        if (*(target = isMobOnMoblist(**curr, &(gm->enemies))) != NULL) {
            freeMob(target);
            freeMob(curr);
            break;
        }
    }
}




//// PLAYER METHODS

/**
 * Player Hit
 * ----------
 * Checks if the player has hit a mob in a given mob collection
 * if so applies damage (or health bonus) to the player and kills the mob
 *
 * @param MobList *ml, reference to the collections of mobs to check
 * @param int damage, value added to the player's health on collision
 *      -> Mob damage is defined above
 */
void playerHit(MobList *ml, int damage) {
    MobList *hit = isMobOnMoblist(*(gm->player), ml);
    if(*hit != NULL) {
        freeMob(hit);
        playerHealth(damage);
    }
}


/**
 * Player Shoot
 * ------------
 * Spawns a player projectile on the player, with a fixed X velocity.
 */
void playerShoot() {
    Mob *tmp = gm->projectiles;
    gm->projectiles = allocMob(PROJECTILE, gm->player->px, gm->player->py);
    gm->projectiles->next = tmp;
    gm->projectiles->vx = 0.001;
}


/**
 * Change Player Speed
 * -------------------
 * Nudges the player's velocity vector by increments.
 * Checks if the velocity is'nt exceeding maximum speed.
 *
 * @param float vx, X axis incrementation
 * @param float vy, Y axis incrementation
 */
void changePlayerSpeedBy(float vx, float vy) {
    if (fabs(gm->player->vx + vx) < MAX_SPEED) gm->player->vx += vx;
    if (fabs(gm->player->vy + vy) < MAX_SPEED) gm->player->vy += vy;
}

/**
 * Player Health
 * -------------
 * Updates the player's health
 * Set game status to 'game over' if health reaches 0
 *
 * @param int health increment
 */
void playerHealth(int value) {
    if ((gm->player->health + value) < 0) {
        gm->level->status = STATUS_GAME_OVER;
    } else {
        gm->player->health += value;
    }
}





/**
 * Ennemy Shoot
 * ------------
 * Spawns an ennemy projectile on a mob, with a fixed negative X velocity.
 * @param Mob e (ennemy), projectile source.
 */
void ennemyShoot(Mob e) {
    if (e.type != ENEMY) return; // only enemies can shoot
    Mob *tmp = gm->ennemyProjectiles;
    gm->ennemyProjectiles = allocMob(ENNEMY_PROJECTILE, e.px, e.py);
    gm->ennemyProjectiles->next = tmp;
    gm->ennemyProjectiles->vx = -0.001;
}


/**
 * Is Mob Hidden
 * -------------
 * Tells if Mobs are on the viewport or not.
 * out of frame mobs should not be drawn or computed in collisions and physics updates.
 *
 * @param Mob m (mob) to check.
 * @return int 0 if in frame, 1 if out of bounds
 */
int isHidden(Mob m) {
    float level_screen_size = (getViewportWidth()/(float)getViewportHeight()) * gm->level->height;
    float zone_size = level_screen_size/(float)gm->level->width;
    
    // mob is off the left bound of the viewport
    int isAfter = m.px < gm->level->progress;
    
    // mob is off the right bound of the viewports
    int isBefore = m.px > gm->level->progress + zone_size;
    
    return isBefore || isAfter;
}
