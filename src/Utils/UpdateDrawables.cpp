//
// Created by Henrique on 6/2/18.
//

#include "UpdateDrawables.h"
#include "DrawSystem.h"
#include "Entity.h"

using namespace ecs;

IDrawable::IDrawable( DrawTarget* iDrawTarget ){


    iDrawTarget->addDrawable( this );
}

void IDrawable::setDrawTarget(ecs::DrawTarget *iDrawTarget){
    
    if( drawTargetOwner )
        drawTargetOwner->removeDrawable( _listPosition );
    
    
    if( iDrawTarget ){
        iDrawTarget->addDrawable(this);
    }else{
        // is draw target is null, remove from current target
        drawTargetOwner->removeDrawable(_listPosition);
        drawTargetOwner = nullptr;
    }
    
}

IDrawable::IDrawable() {
    
    setDrawTarget( DrawSystem::getDefaultDrawTarget().get() );
}


IDrawable::~IDrawable(){
    
    if( drawTargetOwner ){
        drawTargetOwner->removeDrawable( _listPosition );
    }
}

