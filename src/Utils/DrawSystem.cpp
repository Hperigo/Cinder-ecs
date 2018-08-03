//
//  DrawSystem.cpp
//  EcsSerialization
//
//  Created by Henrique on 7/10/18.
//


#include "DrawSystem.h"


std::vector<std::shared_ptr<ecs::DrawTarget>> ecs::DrawSystem::mDrawTargets = std::vector<std::shared_ptr<ecs::DrawTarget>>();
