//
//  DrawSystem.h
//  EcsSerialization
//
//  Created by Henrique on 7/10/18.
//

#ifndef DrawSystem_h
#define DrawSystem_h


#include "UpdateDrawables.h"
#include "ecs/System.h"

#include "cinder/Signals.h"

namespace ecs{
    
    // simple draw target
    struct DrawTarget{
        
        virtual void update(){
            // implement render strategy, z sort, added order sort etc...
        }
        
        
        // actually draw stuff
        virtual void draw(){
            
            // set matrices, bind FBO etc...
            for(auto d : mDrawables){
                d->draw();
            }
        }
        
        void addDrawable( IDrawable* iDrawable ){
            
            iDrawable->drawTargetId = mDrawables.size();
            iDrawable->drawTargetOwner = this;
            mDrawables.push_back(iDrawable);
            
            iDrawable->_listPosition =  -- ( mDrawables.end() );
        }
        void removeDrawable( std::list<IDrawable*>::iterator& itPos ){
            if( itPos != mDrawables.end() ){
                auto obj = *itPos;
                mDrawables.erase( itPos );
                obj->_listPosition = mDrawables.end();
            }
        }
        std::list<IDrawable*> mDrawables;
    };
    
    struct DrawSystem : public ecs::System{
        
        DrawSystem(){
            drawable = true;
            
            auto t = std::make_shared<DrawTarget>();
            mDrawTargets.push_back(t);
        }
        
        
        void update() override{   }
        
        void draw() override{
            
            for(auto d : mDrawTargets){
                d->draw();
            }
            
        }
        
        void addDrawTarget( std::shared_ptr<DrawTarget> iDrawTarget ){
            mDrawTargets.push_back( iDrawTarget );
        };
        
        
        static std::shared_ptr<DrawTarget> getDefaultDrawTarget(){
            
            if( mDrawTargets.size() == 0 ){
                auto t = std::make_shared<DrawTarget>();
                mDrawTargets.push_back(t);
            }
            
            return mDrawTargets[0];
        }
        
        static std::vector <std::shared_ptr<DrawTarget>> mDrawTargets;
    };
}
#endif /* DrawSystem_h */
