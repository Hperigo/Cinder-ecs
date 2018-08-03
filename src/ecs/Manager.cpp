//
//  Manager.cpp
//  EcsCereal
//
//  Created by Henrique on 6/21/18.
//

#include "Manager.h"

using namespace ecs;
std::map< std::string , std::shared_ptr<ecs::internal::ComponentFactoryInterface>> Manager::typeFactory = std::map< std::string, std::shared_ptr<ecs::internal::ComponentFactoryInterface > >();
