//
//  Button.h
//  EntityUi
//
//  Created by Henrique on 1/9/18.
//

#ifndef Button_h
#define Button_h

#include "ecs/Component.h"
#include "ecs/System.h"
#include "Utils/Transform.h"

#include "cinder/Rect.h"


#include "cinder/app/MouseEvent.h"
#include "cinder/Log.h"

struct Bounds  : public ecs::Component {
    
    Bounds() {
        
        onUpdateSignal = std::make_shared< ci::signals::Signal< void() > >();
        
    }
    Bounds(const ci::Rectf& iRect ) : rect ( iRect ){
        onUpdateSignal = std::make_shared< ci::signals::Signal< void( ) > >();
    }
    
    void setBound(const ci::Rectf& iRect, bool emmitSignal = true ){
        rect = iRect;

        
        if( onUpdateSignal->getNumSlots() != 0 && emmitSignal ){
            onUpdateSignal->emit();
        }
    }
    
    
    std::shared_ptr< ci::signals::Signal< void() > >  onUpdateSignal;
    ci::Rectf rect{0,0,100,10 };
};




struct Button  : public ecs::Component {

    Button(){
    }

    void setup() override {
        
        getEntity().lock()->addComponent<Bounds>();
        
        getEntity().lock()->addComponent<Transform>();

    }
    
    std::function<void(const ci::app::MouseEvent& event )> onMouseUp;
    std::function<void(const ci::app::MouseEvent& event )> onMouseDrag;
    std::function<void(const ci::app::MouseEvent& event )> onMouseDown;
    
    
    enum State{
        MOUSE_NONE,
        MOUSE_INSIDE,
        MOUSE_DRAG,
        MOUSE_DOWN,
        MOUSE_UP,
        MOUSE_OVER
    };

    State currentState = MOUSE_NONE;
    
    bool intercept = false;
};


//struct CDraggable : public entityx::Component<CDraggable>{
//
//    std::function<void()> onMouseUp;
//    std::function<void()> onMouseDown;
//    std::function<void()> onMouseDrag;
//
//    enum State{
//        NONE,
//        DRAGGING
//    };
//
//    State currentState = State::NONE;
//};



struct MouseInputSystem : public ecs::System {
 
    std::vector< ecs::EntityRef > mActiveEntities;
    
    

    void setup() override {
        
        ci::app::App::get()->getWindow()->getSignalMouseDown().connect( std::bind( &MouseInputSystem::mouseDown, this, std::placeholders::_1 ) );
        ci::app::App::get()->getWindow()->getSignalMouseDrag().connect( std::bind( &MouseInputSystem::mouseDrag, this, std::placeholders::_1 ) );
        ci::app::App::get()->getWindow()->getSignalMouseUp().connect( std::bind( &MouseInputSystem::mouseUp, this, std::placeholders::_1 ) );
        ci::app::App::get()->getWindow()->getSignalMouseMove().connect( std::bind( &MouseInputSystem::mouseMove, this, std::placeholders::_1 ) );
        
    }
    
    
    void mouseDown( const ci::app::MouseEvent& iEvent  ){
        
        auto entities = getManager()->getEntitiesWithComponents<Button>();
        
        std::reverse(entities.begin(), entities.end());
        
        
        for(  auto& e : entities){

            
            auto b = e->getComponent<Bounds>();
            auto button =  e->getComponent<Button>();
            auto transform = e->getComponent<Transform>();
 
            ci::vec2 transformedPoint;
            ci::mat4 inverdedMatrix  = glm::inverse(transform->getWorldTransform());
            auto p = inverdedMatrix * glm::vec4( iEvent.getX(), iEvent.getY(), 0.0f, 1.0f);
            transformedPoint.x = p.x;
            transformedPoint.y = p.y;
            
            
            if(  b->rect.contains( transformedPoint ) ){
                
                if( button->onMouseDown && !e->isActive() )
                    button->onMouseDown( iEvent );
                
                button->currentState = Button::State::MOUSE_DOWN;
                
                if(button->intercept){
                    break;
                }
            }
        }
    }
    
    void mouseMove( const ci::app::MouseEvent& iEvent  ){
        
        auto entities = getManager()->getEntitiesWithComponents<Button>();
        
        std::reverse(entities.begin(), entities.end());
        
        
        for(  auto& e : entities){
            
            
            auto b = e->getComponent<Bounds>();
            auto button =  e->getComponent<Button>();
            auto transform = e->getComponent<Transform>();
            
            ci::vec2 transformedPoint;
            ci::mat4 inverdedMatrix  = glm::inverse(transform->getWorldTransform());
            auto p = inverdedMatrix * glm::vec4( iEvent.getX(), iEvent.getY(), 0.0f, 1.0f);
            transformedPoint.x = p.x;
            transformedPoint.y = p.y;
            
            button->currentState = Button::State::MOUSE_NONE;
            
            if(  b->rect.contains( transformedPoint ) ){
                
                button->currentState = Button::State::MOUSE_OVER;
                
                if(button->intercept){
                    break;
                }
            }
        }
        
        
    }
    
    void mouseDrag( const ci::app::MouseEvent& iEvent  ){
        
        
        
    }
    
    void mouseUp( const ci::app::MouseEvent& iEvent  ){

        for(  auto& e : getManager()->getEntitiesWithComponents<Button>()){
            
            if( e->isActive() == false ){
                continue;
            }
            
            auto b = e->getComponent<Bounds>();
            auto button =  e->getComponent<Button>();
            
            if(  button->currentState == Button::State::MOUSE_DOWN ){
                if( button->onMouseUp )
                    button->onMouseUp(iEvent);
            }
            
            button->currentState = Button::State::MOUSE_NONE;
            
            
            
        }
        
    }
    
    
    void draw() override {
        
        for(  auto& e : getManager()->getEntitiesWithComponents<Button>()){
            
                ci::Color color(1.0f, 0.0f, 0.0f);
            
            auto button = e->getComponent<Button>();
            auto transform = e->getComponent<Transform>();
            auto bounds = e->getComponent<Bounds>();

            
                if( button->currentState == Button::State::MOUSE_DOWN  ){
                    color = ci::Color( 1.0f, 1.0f, 0.0f );
                }

                ci::gl::ScopedModelMatrix sM;
                ci::gl::ScopedColor sStateColor( color );
                ci::gl::setModelMatrix(  transform->getWorldTransform() );
                ci::gl::drawStrokedRect( bounds->rect );
            
        }
    }
};


/*
struct MouseDragSystem : public entityx::System<MouseDragSystem> {
    
private:
    entityx::EntityX& _entities;
    
    struct DraggedEntity {
        DraggedEntity() { };
        
        DraggedEntity( CTransform::Handle iDragged, CTransform::Handle iParent, ci::vec3 iPos ) :
        dragged(iDragged),
        parent(iParent),
        initialPos(iPos)
        {  };
        
        
        CTransform::Handle dragged;
        CTransform::Handle parent;
        
        
        
        ci::vec3 initialPos;
    };
    std::vector< DraggedEntity > activeEntities;
    ci::vec3 initialMousePos;
    
public:
    MouseDragSystem(entityx::EntityX& iEntity) : _entities( iEntity ) {  } ;
    
    void configure( entityx::EventManager &events ) override{
        
        ci::app::App::get()->getWindow()->getSignalMouseDown().connect( std::bind( &MouseDragSystem::mouseDown, this, std::placeholders::_1 ) );
        ci::app::App::get()->getWindow()->getSignalMouseUp().connect( std::bind( &MouseDragSystem::mouseUp, this, std::placeholders::_1 ) );
        ci::app::App::get()->getWindow()->getSignalMouseDrag().connect( std::bind( &MouseDragSystem::mouseDrag, this, std::placeholders::_1 ) );
    }
    
    void update(entityx::EntityManager &entities, entityx::EventManager &events, entityx::TimeDelta dt) override { }

    
    
    void mouseDown( const ci::app::MouseEvent& iEvent ){
        
        
        ci::app::console() << "down..." << std::endl;
        
        
        initialMousePos = ci::vec3(iEvent.getPos(), 0);
        _entities.entities.each< Bounds, CDraggable >([this, iEvent]( entityx::Entity entity, Bounds& bounds, CDraggable& draggable) {
            
            auto trans = entity.component<CTransform>();
            ci::vec2 transformedPoint;
            if( trans ){
                
                ci::mat4 inverdedMatrix  = glm::inverse(trans->getWorldTransform());
                auto p = inverdedMatrix * glm::vec4( iEvent.getX(), iEvent.getY(), 0.0f, 1.0f);
                transformedPoint.x = p.x;
                transformedPoint.y = p.y;
                
            }else{
                transformedPoint = iEvent.getPos();
            }
            

            
            bool contains = bounds.rect.contains( transformedPoint );
            if( contains ){
                

                DraggedEntity dragged;
                
                if( trans->hasParent() ){
                    dragged = DraggedEntity( trans->handle(), trans->getParent()->handle(), trans->getWorldPosition() );
                    trans->removeParent(true);
                }else{
                    dragged = DraggedEntity( trans->handle(), CTransform::Handle(), trans->getWorldPosition() );
                }
                
                activeEntities.push_back( dragged );
            }
            
        });
    }
    
    
    void mouseDrag( const ci::app::MouseEvent& iEvent){
        
        for( auto active : activeEntities ){
            
            ci::vec3 offset =  ci::vec3(iEvent.getPos().x, iEvent.getPos().y, 0.0f) - initialMousePos;
            active.dragged->setWorldPosition( (active.initialPos + offset) );
            
            auto entity = active.dragged->getOwner();
            entity.component<CDraggable>()->currentState = CDraggable::DRAGGING;
        }
    }
    
    void mouseUp( const ci::app::MouseEvent& iEvent ){
    
        
        for(auto active :  activeEntities){
            
            
            if( active.parent != CTransform::Handle() ){
                active.dragged->setParent( active.parent );
            }
            auto entity = active.dragged->getOwner();
            entity.component<CDraggable>()->currentState = CDraggable::State::NONE;
        }
        
        activeEntities.clear();
    }

    
    void draw(){
        
        
        {
            
            ci::gl::ScopedColor  sc( ci::Color(1.0f, 1.0f, 0.0f) );
            
            for(auto active :  activeEntities){
                ci::gl::drawStrokedCircle(active.initialPos, 5.0f);
            }
            
        }
        
        _entities.entities.each< Bounds, CDraggable, CTransform >([this]( entityx::Entity entity, Bounds& bounds, CDraggable& button, CTransform& trans) {
            {
                ci::Color color(0.0f, 0.0f, 1.0f);
                
                if( button.currentState == CDraggable::State::DRAGGING ){
                    color = ci::Color( 1.0f, 1.0f, 0.0f);
                }
                
                ci::gl::ScopedModelMatrix sM;
                ci::gl::ScopedColor sStateColor( color );
                ci::gl::setModelMatrix( trans.getWorldTransform() );
                ci::gl::drawStrokedRect( bounds.rect );
            }
        });
    }
    
};*/

#endif /* Button_h */
