//
//  Particles.h
//  EcsRendering
//
//  Created by Henrique on 7/19/18.
//

#ifndef Particles_h
#define Particles_h

#include "cinder/Rand.h"
#include "cinder/gl/gl.h"


#include "cinder/Timer.h"

using namespace ci;

struct Particle : ecs::Component{
    
    Particle(){
        
    }
    
    Particle( ci::vec2 iPos, ci::vec2 iSpeed ) : pos( iPos ), speed(iSpeed){  }
    
    ci::vec2 speed;
    ci::vec2 pos;
    float lifetime = ci::Rand::randFloat(5, 20);
    
};



struct ParticleSystem : public ecs::System, public ecs::IDrawable{
    
    void setup() override{
        setDrawable(false);
        
        
    }
    
    void update() override{

        ci::Timer t;
        t.start();
        auto ent = getManager()->getComponentsArray<Particle>();
        
        for(auto& e : ent ){
            
            auto particleHandle = e;
            
            particleHandle->speed *= 0.94;
            particleHandle->pos   += particleHandle->speed;
            
            particleHandle->lifetime -= 0.1;
            
            if( particleHandle->lifetime < 0 ){
                auto entHandle = particleHandle->getEntity().lock();
                particleHandle->getEntity().lock()->destroy();
            }
        }
    }
    
    
    void draw() override {
        
        for(auto& e : getManager()->getEntitiesWithComponents<Particle>() ){

            auto particle = e->getComponent<Particle>();
            gl::ScopedModelMatrix m;
            gl::translate( particle->pos );

            float size = particle->lifetime;
            gl::drawSolidRect(Rectf(-size,-size, size, size));

        }
        
    }
    
    void addParticle( ci::vec2 pos ){
        auto e = getManager()->createEntity();
        e->addComponent<Particle>( pos, Rand::randVec2() * 7.f );
    }

};



#endif /* Particles_h */
