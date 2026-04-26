#ifndef Effects_h__
#define Effects_h__

#include "LTE/Common.h"
#include "Game/Common.h"

void Effect_BeamHit(
  Position const& origin,
  V3 const& baseVelocity,
  float scale,
  V3 const& color);

void Effect_MultiExplosionRadial(
  Object const& object,
  float scale,
  ExplosionType type);

void Effect_ParticleFirefly(
  Position const& origin,
  V3 const& velocity,
  V3 const& color,
  float size,
  float lifeTime);

void Effect_SmallPlume(
  Position const& origin,
  V3 const& baseVelocity,
  V3 const& color,
  float scale);

#endif
