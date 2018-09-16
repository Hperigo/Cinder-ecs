#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "ecs/Manager.h"
#include "CinderImGui.h"


using namespace ci;
using namespace ci::app;
using namespace std;



class CustomEntity : public ecs::Entity, public ecs::IDrawable {

public:
	void setup() override {

		setDrawTarget(ecs::DrawSystem::getInstance()->getDefaultDrawTarget() );

		this->addComponent<vec2>();

	}

	void draw() override {

		gl::ScopedModelMatrix sm;
		gl::translate(*getComponent<vec2>());
		gl::drawSolidRect({ -10, -10, 10, 10 });

	}

};


class EcsTestsApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	ecs::ManagerRef mManager;
	ecs::EntityRef  mEntity;
};

void EcsTestsApp::setup()
{

	mManager = ecs::Manager::create();

	mEntity = mManager->createEntity<CustomEntity>();


	mManager->setup();
}

void EcsTestsApp::mouseDown( MouseEvent event )
{

	mEntity->getComponent<vec2>()->x = event.getX();
	mEntity->getComponent<vec2>()->y = event.getY();

}

void EcsTestsApp::update()
{
	mManager->update();

}

void EcsTestsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 


	{
		mManager->draw();
	}
}

CINDER_APP( EcsTestsApp, RendererGl )
