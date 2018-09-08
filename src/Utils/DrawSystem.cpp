//
//  DrawSystem.cpp
//  EcsSerialization
//
//  Created by Henrique on 7/10/18.
//


#include "DrawSystem.h"


using namespace ecs;
ecs::DrawSystem* ecs::DrawSystem::mInstance = nullptr;

DrawSystem* ecs::DrawSystem::getInstance() {
	if (mInstance == nullptr) {
		static DrawSystem mS;
		mInstance = &mS;
	}

	return mInstance;
}


IDrawable::IDrawable() {

	setDrawTarget(DrawSystem::getInstance()->getDefaultDrawTarget());
}

IDrawable::IDrawable( DrawTarget* iDrawTarget ){
    
    
    iDrawTarget->addDrawable( this );
}

void IDrawable::setDrawTarget( std::shared_ptr<ecs::DrawTarget> iDrawTarget){
    
    if( drawTargetOwner )
        drawTargetOwner->removeDrawable( _listPosition );
    
    
    if( iDrawTarget ){
        iDrawTarget->addDrawable(this);
        
    }else{ //if draw target is null, remove it from owner
        
        
        if( hasDrawTarget() == false ){
            return;
        }
        
        // is draw target is null, remove from current target
        drawTargetOwner->removeDrawable(_listPosition);
        drawTargetOwner = nullptr;
    }
    
}




IDrawable::~IDrawable(){
    
    if( hasDrawTarget() ){
        drawTargetOwner->removeDrawable( _listPosition );
    }
}
