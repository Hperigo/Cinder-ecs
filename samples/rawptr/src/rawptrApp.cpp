#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"

#include "ecs/Manager.h"

#include "Utils/TransformSystem.h"

#include "CinderImGui.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class rawptrApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void keyUp( KeyEvent event ) override;
	void update() override;
	void draw() override;
    
    
    ecs::ManagerRef mManager;
    
};

void rawptrApp::setup()
{
    mManager = ecs::Manager::create();

    
    for( int i = 0; i < 1000; i++ ){
        
        auto e = mManager->createEntity();
        auto t = e->addComponent<Transform>();
        t->setPos( { randFloat(0, getWindowWidth()), randFloat(0, getWindowHeight()), 0 } );
        t->setAlwaysUpdate(true);
    }
    
    
    mManager->createSystem<TransformSystem>()->setDrawable(false);
    mManager->setup();
    
    ui::initialize();
}

void rawptrApp::mouseDown( MouseEvent event )
{
//    mEntity->getComponent<Transform>()->setPos( vec3(event.getPos(), 0.f) );
}

void rawptrApp::keyUp( KeyEvent event ){
    
//    mEntity->getComponent<Transform>()->removeChild(mChild->getComponent<Transform>());
    
}

void rawptrApp::update()
{
    
    getWindow()->setTitle( to_string(getAverageFps() ) );
    
    mManager->update();
    
}

void rawptrApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    
    mManager->draw();
}

CINDER_APP( rawptrApp, RendererGl )
