#ifndef MC2D_PHYSICS_H
#define MC2D_PHYSICS_H

#include "../mc2d_types.h"
#include "../world/worldgen.h"

/* Physics context — stateless, uses WorldCtx for collision queries */
void physics_update(Entity* e, WorldCtx* w, float dt);

/* AABB vs world sweep — returns true if collided */
bool physics_sweep_x(WorldCtx* w, AABB* box, float* vx, float dt);
bool physics_sweep_y(WorldCtx* w, AABB* box, float* vy, float dt);

/* Utility */
bool physics_overlaps_block(WorldCtx* w, float wx, float wy);
bool aabb_overlap(const AABB* a, const AABB* b);

#endif /* MC2D_PHYSICS_H */
