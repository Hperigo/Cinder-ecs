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


struct Test : ecs::Component {
    
    
    
};

struct Particle : ecs::Component{
    
    Particle(){
        
    }
    
    Particle( ci::vec2 iPos, ci::vec2 iSpeed ) : pos( iPos ), speed(iSpeed){  }
    
    ci::vec2 speed;
    ci::vec2 pos;
    float lifetime = 10;
    
    
    void onDestroy() override {
        
        auto e = getEntity().lock();
        e->removeComponent<Test>();
    }
};



struct ParticleSystem : public ecs::System, public ecs::IDrawable{
    
    void setup() override{
        setDrawable(false);
        
        
    }
    
    void update() override{
        mParticles =  getManager()->getEntitiesWithComponents<Particle>();
        
        for(auto& e : mParticles ){
            
            auto particleHandle = e->getComponent<Particle>();
            
            particleHandle->speed *= 0.94;
            particleHandle->pos   += particleHandle->speed;
            
            particleHandle->lifetime -= 0.1;
            
            if( particleHandle->lifetime < 0 ){
                e->destroy();
            }
        }
        
    }
    
    
    void draw() override {
        
        for(auto& e : mParticles ){
            
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
        e->addComponent<Test>();
    
    }
    
    std::vector<ecs::EntityRef> mParticles;
};



#endif /* Particles_h */
