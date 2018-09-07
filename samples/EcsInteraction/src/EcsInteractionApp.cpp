#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "ecs/Manager.h"
#include "Utils/TransformSystem.h"

#include "cinder/Timeline.h"

#include "CinderImgui.h"


#include "MouseInteraction.h"

using namespace ci;
using namespace ci::app;
using namespace std;


struct Info{
    
    
    Info(){
        name = Color::black();
    }
    
    
    void makeWhite(){
        name = Color::white();
    }
    
    Color name;
};


class SceneButton : public ecs::Entity, public ecs::IDrawable {    
    public:
    
    
    std::shared_ptr<SceneButton> build( float w ){
        auto bt = addComponent<Button>();
        bt->intercept = true;
        getComponent<Bounds>()->rect = Rectf(0,0, w, 50);
        return  static_pointer_cast<SceneButton>(shared_from_this());
    }

    void draw()  override {
        
        Color c(1.0f, 0.8f, 0.8f );
        
        if( getComponent<Button>()->currentState == Button::State::MOUSE_DOWN ){
            c *= 1.2f;
        }
        
        if( getComponent<Button>()->currentState == Button::State::MOUSE_OVER ){
            c *= 1.1f;
        }
        
        
        if( !isActive() ){
            c *= 0.1;
        }
        
        gl::ScopedColor sc(c);
        auto r =  getComponent<Bounds>();
        gl::setModelMatrix( getComponent<Transform>()->getWorldTransform() );
        gl::drawSolidRect( r->rect );
        
    }
};


class SceneTab : public ecs::Entity, public ecs::IDrawable{
    
public:
    void setup() override{

        auto mainBt = addComponent<Button>();
        mainBt->onMouseUp = [&]( const MouseEvent& event ){
            
            bool h = !isHidden;
            console() << h << endl;
            toogle(h);
            
            for( auto& b : mButtons ){
                b->setActive( !h );
            }
            
        };
        getComponent<Bounds>()->rect = Rectf(0, 0, 400, getWindowHeight() );
        
        getWindow()->getSignalResize().connect( [&]{
            getComponent<Bounds>()->setBound( Rectf(0, 0, 400, getWindowHeight()) );
        });
        
        auto w = getComponent<Bounds>()->rect.getWidth();

        
        for( int i = 0; i < 5; i++ ){
            
            auto bt = getManager()->createEntity<SceneButton>()->build(w);
            bt->getComponent<Transform>()->setParent( getComponent<Transform>() );

            bt->getComponent<Transform>()->setPos( vec3( 0, 100 + i * 55, 0.f) );
            
            bt->getComponent<Button>()->onMouseUp = [&, i](const ci::app::MouseEvent& event){
                
                timeline().applyPtr(&radiusScale, 0.0f, 1.0, EaseInOutAtan() ).finishFn( [&] { mCurrentState = State::RECT; currentId = i;  } );
                timeline().appendToPtr(&radiusScale, 4.0f, 1.0, EaseOutAtan() );
                
            };
            
            mButtons.push_back( bt );
        }
        
    }
    
    
    void draw() override {
        
        { // draw tab
            gl::ScopedModelMatrix sm;
            gl::setModelMatrix( getComponent<Transform>()->getWorldTransform() );
            gl::ScopedColor sc( 0.2, 0.5, 0.7f );
            
            auto r =  getComponent<Bounds>();
            gl::drawSolidRect( r->rect );
        }
        
        { // draw shapes

            vec2 pos = {getWindowWidth() - 80, getWindowHeight() * 0.5 };
            
            gl::ScopedColor sc( 0.8f, 0.3f, 0.0f );
            if( mCurrentState == State::CIRCLE ){
                gl::drawSolidCircle( pos, 50 * radiusScale);
            }
            else{
                
                Rectf r(-30 * radiusScale,-30 * radiusScale, 30 * radiusScale, 30 * radiusScale );
                r.offsetCenterTo( pos  );
                gl::drawSolidRect( r );
                gl::drawString( std::to_string(currentId),  r.getUpperLeft() + vec2(10,20) );
            }
        }
        
        
    }
    
    
    void toogle( bool hide ){
        
        auto t = getComponent<Transform>();
        if(  hide )
            timeline().applyPtr( t->getPosPtr(), vec3(-340, 0, 0), 0.3f, EaseInOutQuad() ).updateFn([&, t]{ t->updateMatrices(); });
        else
            timeline().applyPtr( t->getPosPtr(), vec3(0, 0, 0), 0.3f, EaseInOutQuad() ).updateFn([&, t]{ t->updateMatrices(); });
        
        
        isHidden = hide;
    }
    


protected:
    
    
    size_t currentId;
    
    std::vector<ecs::EntityRef> mButtons;
    
    bool isHidden = false;
    float radiusScale = 1.0f;
    
    enum class State{
        CIRCLE,
        RECT
    };
    State mCurrentState;
    
};


class EcsInteractionApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    ecs::ManagerRef mManager;
    ecs::EntityRef mEntity;
    
    
    std::shared_ptr<SceneTab> mTab;
    
    float value = .0f;
};

void EcsInteractionApp::setup()
{
    
    mManager = ecs::Manager::create();
    mManager->createSystem< TransformSystem>()->setDrawable(false);

    auto s = mManager->createSystem<MouseInputSystem>();
    s->setDrawable(false);

    mTab = mManager->createEntity<SceneTab>();
    console() << *mTab->addComponent<Color>() << endl;;

    mManager->setup();
    

}

void EcsInteractionApp::mouseDown( MouseEvent event )
{

}

void EcsInteractionApp::update()
{
    mManager->update();
   
}

void EcsInteractionApp::draw()
{
    gl::clear( Color::gray(0.2) );
    gl::setMatricesWindow( getWindowSize() );
    
    
    mManager->draw();
}

CINDER_APP( EcsInteractionApp, RendererGl )
