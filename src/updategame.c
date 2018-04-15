#include "../include/updategame.h"

void updatePlayer(Mob *p) {
    // position
    p->px += p->vx;
    p->py += p->vy;
    
    // drag
    p->vx *= DRAG;
    p->vy *= DRAG;
    
    
    // bounce on borders
    if (p->px > 1 || p->px < 0) {
        p->vx *= -1;
        p->px = roundf(p->px);
    }
    
    if (p->py > 1 || p->py < 0) {
        p->vy *= -1;
        p->py = roundf(p->py);
    }
}

void updateEnnemy(Mob *e) {
    e->px += 0;
}

void updateProjectile(Mob *p) {
    // position
    p->px += p->vx;
    p->py += p->vy;
}
