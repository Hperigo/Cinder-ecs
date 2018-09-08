

#include "ecs/Manager.h"


using namespace ecs;

std::map< std::string, std::shared_ptr<internal::ComponentFactoryInterface>> Manager::typeFactory = std::map< std::string, std::shared_ptr<internal::ComponentFactoryInterface>>();
