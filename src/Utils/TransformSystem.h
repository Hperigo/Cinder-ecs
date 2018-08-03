//
//  Transform.h
//  LittleECS
//
//  Created by Henrique on 8/21/17.
//
//

#ifndef TransformSystem_h
#define TransformSystem_h

#include "ecs/Manager.h"
#include "Utils/Transform.h"

struct TransformSystem : ecs::System{
    
    void update() override{
        
        auto& transforms = getManager()->getComponentsArray<Transform>();
        for(auto& c : transforms){
            
            auto t = static_pointer_cast<Transform>(c);
            if(t->needsUpdate()){
               t->updateMatrices();
            }
        }
    }
    
    
    void draw() override {
        
        
        auto transforms = getManager()->getComponentsArray<Transform>();

        
        for( auto& handle : transforms ){

            auto trans = static_pointer_cast<Transform>( handle );
            
            // draw parent child relation
            if( trans->hasParent() ){
                
                auto parentHandle = trans->getParent().lock();
                ci::gl::drawLine(trans->getWorldPos(),  parentHandle->getWorldPos() );

            }

            
            {
                // draw AnchorPoint
                ci::gl::ScopedModelMatrix m;
                ci::gl::translate( trans->getWorldPos() );
                gl::drawSolidRect( { -5,-5, 5,5 }  );
            
            }
            
            {
                ci::gl::ScopedModelMatrix m;
                ci::gl::multModelMatrix( trans->getWorldCTransform() );
                ci::gl::translate( trans->getAnchorPoint() );
                const float length = 50;
                // x axis
                ci::gl::color(ci::Color( 1.0f, 0, 0 ));
                ci::gl::drawLine( glm::vec3(0,0, 0), glm::vec3(length, 0, 0) );

                // y axis
                ci::gl::color( ci::Color( 1.0f, 1.0f, 0.0f ) );
                ci::gl::drawLine( glm::vec3( 0,0 ,0), glm::vec3( 0, -length,0 ) );

                ci::gl::color( ci::Color::white());
            }
//
        }
    }
    
    
};


#endif /* TransformSystem_h */
