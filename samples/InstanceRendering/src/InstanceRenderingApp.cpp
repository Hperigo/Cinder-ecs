#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Utils/Factory.h"

#include "Utils/TransformSystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Particle : public ecs::Component{   };


typedef ecs::WrapperComponent<Color> ColorComponent;


class InstanceRenderingApp : public App {
  public:
	void setup() override;
    
    void resize() override;
    
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    
    ecs::Manager mManager;
};

void InstanceRenderingApp::setup()
{
    
    
    
    for( int x = 0; x < 100000; x+=1){
        
        ecs::EntityRef entity = mManager.createEntity();
        entity->addComponent<CTransform>();
        entity->addComponentWrapper<Color>();
    }
    
    
    mManager.createSystem<TransformSystem>();

    
    ecs::factory::initialize(mManager, true, false);
}

void InstanceRenderingApp::mouseDown( MouseEvent event )
{
    
}

void InstanceRenderingApp::resize(){
   
    /*
    for( auto e : mManager.getEntities() ){
        e->destroy();
    }
    
    mManager.refresh();
    
    float numOfX = 3000;
    float numOfY = getWindowHeight() / 40;
    
    for( int x = 0; x < 300; x+=1){
        
            ecs::EntityRef entity = mManager.createEntity();
            //auto t = entity->addComponent<CTransform>().lock();
            //t->setPos( vec3(x, y, 0) );
        }
     */
    
}

void InstanceRenderingApp::update()
{
    
    auto& ts = mManager.getComponentsArray<ColorComponent>();
    getWindow()->setTitle( "fps: " + to_string(getAverageFps()) +  " | "  + std::to_string( ts.size() ) );

    ci::Timer t;
    t.start();

    for(auto& c : ts ){
        auto transform = static_pointer_cast<ColorComponent>(c);
        transform->object = ci::Color::white();
    }
    
    
    console() << t.getSeconds() * 1000 << endl;
    
}

void InstanceRenderingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
//    auto entities =  mManager.getEntitiesWithComponents<CTransform>();
//    for(auto& e : entities){
//
//        auto transform = e->getComponent<CTransform>().lock();
//
//        gl::ScopedModelMatrix sm;
//        gl::multModelMatrix( transform->getCTransformMatrix() );
//        gl::drawSolidRect(Rectf(-10, -10, 10, 10));
//
//    }
}

CINDER_APP( InstanceRenderingApp, RendererGl )
