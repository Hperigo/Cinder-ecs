//
// Created by Henrique on 5/24/18.
//

#include "Entity.h"
#include "Manager.h"

using namespace ecs;



unsigned int Entity::mNumOfEntities = 0;

void Entity::addComponentToManager( ComponentID cId, ComponentRef component){

    mManager->addComponent( cId, component );

    mComponentArray[cId] = component.get();
    mComponentBitset[cId] = true;

    component->mEntity = shared_from_this();
    component->mManager = mManager;
    component->mComponentId = cId;
    component->setup();

}

void Entity::markRefresh(){
    mManager->needsRefresh = true;
}





