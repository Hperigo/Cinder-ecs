#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Path2d.h"

#include "CinderImGui.h"

#include "Utils/Factory.h"

#include "DrawTargets.h"

// add a particle system in the Fbo target, it needs to be set as not "drawable" other wise the system will be drawn twice, in the fbo and the default draw call in the manager
#include "Particles.h"

using namespace ci;
using namespace ci::app;
using namespace std;


// Some basic components
struct  ColorComponent{
    ci::Color _color;
};

struct RectComponent : public ecs::Component, public ecs::IDrawable{
    
    ci::Rectf _r;
    
    RectComponent() : ecs::IDrawable(){
        
        _r = Rectf(-50,-50, 50, 50 );
    }
    
    RectComponent( std::shared_ptr<ecs::DrawTarget> iDrawTarget) : ecs::IDrawable( iDrawTarget.get() ){
        
        _r = Rectf(-50,-50, 50, 50 );
    }
    
    
    void draw() override{
        
        auto entity = getEntity().lock();
        
        if(!entity){
            return;
        }
        gl::ScopedModelMatrix m;
        
        auto c = entity->getComponent<Transform>();
        gl::multModelMatrix(c->getWorldTransform());
        
        auto color = Color::white();
        
        if( entity->hasComponent<  ecs::WrapperComponent<Color>> () ){
            color = entity->getComponent< ecs::WrapperComponent<Color>>()->object;
        }
        
        gl::ScopedColor sc( color );
        gl::drawSolidRect( _r );
    }
    
    void onDestroy() override{
        
        console() << ". " << endl;
    }
};


struct TextureComponent : public ecs::Component, public ecs::IDrawable{
    
    TextureComponent() {  }
    
    TextureComponent( std::shared_ptr<ecs::DrawTarget> iDrawTarget) : ecs::IDrawable( iDrawTarget.get() ){   }
    
    void draw(){
        
        auto entity = getEntity().lock();
        
        if(entity){ // draw the generated texture
            gl::ScopedModelMatrix m;
            auto c = entity->getComponent<Transform>();
            gl::multModelMatrix(c->getWorldTransform());
            gl::ScopedColor sc( ColorA(1.0f, 1.0f, 1.0f, 1.0f) );
            gl::draw( mTexture, Rectf(0,0, 200,200) );
            gl::drawString( "texture from FBO beein drawn in a texture component", vec2(10,10) );
        }
    }

    gl::TextureRef mTexture;
};

/// ------ end of components ------


class MyCustomEntity : public ecs::Entity, public  ecs::IDrawable {
    
public:
    MyCustomEntity( float radius ) : mRadius( radius ), ecs::IDrawable() {   }
    
	~MyCustomEntity()
	{
		
	}
    
    void setup() override {
        
        App::get()->getSignalUpdate().connect( std::bind( &MyCustomEntity::customUpdateCall, this) );
        addComponent<Transform>();
    }
    
    
    void customUpdateCall() {
        mRadius = 10 * (sin(getElapsedFrames() * 0.1) + 2 * 0.5f);
    }
    
    
    void draw() override {
        
        gl::ScopedModelMatrix m;
        
        auto t = getComponent<Transform>();
        
//        gl::multModelMatrix( t->getTransformMatrix() );
        gl::color( Color(0.4, 0.4, 0.7) );
        gl::drawSolidCircle( t->getPos() , mRadius );
        
    }
    
    
    float mRadius = 1.0f;
};

class EcsRenderingApp : public App {
  public:
	void setup() override;
	void mouseMove( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    
    ecs::Manager mManager;
    
    
    // entities ----
    ecs::EntityRef mEntity;
    ecs::EntityRef mEntityB;
    std::shared_ptr<MyCustomEntity> mCustom;

    // systems -----
    std::shared_ptr<ParticleSystem> mParticleSystem;
    
    // draw targets ---
    std::shared_ptr<FboDrawTarget> mFboDrawTarget;
    std::shared_ptr<BlurFboDrawTarget> mBlurDrawTarget;
    std::shared_ptr<ecs::DrawTarget> mDefaultDrawTarget;

};

void EcsRenderingApp::setup()
{

    // create and add the FBO draw target to the default manager draw system. The draw system is just a collection of draw targets, you can have a Fbo draw target, scissor draw target or pass-trought
    mFboDrawTarget = std::make_shared<FboDrawTarget>();
    ecs::DrawSystem::getInstance()->addDrawTarget( mFboDrawTarget );

    
    mBlurDrawTarget = std::make_shared<BlurFboDrawTarget>( vec2( 500, 500 ), 0.5f );
    mBlurDrawTarget->setClearColor( ColorA::gray(0.91f, 0.f) );
    ecs::DrawSystem::getInstance()->addDrawTarget( mBlurDrawTarget );
    

    { // FBO target ------

        // add a particle system to the draw target
        mParticleSystem = mManager.createSystem<ParticleSystem>();
        mParticleSystem->setDrawTarget( mBlurDrawTarget );
        
        // create a rotating rect for fun
        mEntity = mManager.createEntity();
        mEntity->addComponent<Transform>()->setPos(vec3(200,200,0));
        mEntity->addComponent<RectComponent>( mBlurDrawTarget );
        mEntity->addComponent<ci::Color>( 0.1f, 0.1f, 0.2f );

    }

    // create a second entity that will draw the result texture from the FboDrawTarget
    {
        
        mDefaultDrawTarget = std::make_shared<ecs::DrawTarget>();
        ecs::DrawSystem::getInstance()->addDrawTarget( mDefaultDrawTarget );
        
        mEntityB = mManager.createEntity();
        mEntityB->addComponent<Transform>()->setPos(vec3(getWindowCenter(),0));
        
        mEntityB->addComponent<TextureComponent>();
        mEntityB->getComponent<TextureComponent>()->mTexture = mBlurDrawTarget->getBluredFbo()->getColorTexture();
        
        mCustom = mManager.createEntity<MyCustomEntity>(100);
    }
    
//    ecs::factory::initialize(mManager);
    
    ui::initialize();
    
    console() << "initialized!" << "💊" << std::endl;
    
    
    mManager.setup();
}

void EcsRenderingApp::mouseMove( MouseEvent event )
{
    if( mEntityB ){
        mEntityB->getComponent<Transform>()->setPos( vec3( event.getPos(), 0.f ) );
    }
    
        mCustom->getComponent<Transform>()->setPos(vec3( event.getPos(), 0.f ));
}

void EcsRenderingApp::update()
{
    mEntity->getComponent<Transform>()->setRotation(getElapsedFrames()*0.05f);
    
    
    if( getElapsedFrames() % 10 == 0 ){
        
        mParticleSystem->addParticle( { 200,200 } );
    }
    
    ui::DragFloat( "blur amt",  &mBlurDrawTarget->blurAmt, 0.01f );
    ui::DragFloat( "att",  &mBlurDrawTarget->attenuation, 0.01f );
    
    mManager.update();
    
}

void EcsRenderingApp::draw()
{

    gl::clear( Color::gray(0.91f) );
    gl::ScopedViewport sv( getWindowSize() );
    gl::setMatricesWindow( getWindowSize() );
    
    gl::enableAlphaBlending();
    
    mManager.draw();

    {
//        gl::color(ColorA(1.0f,1.0f,1.0f, 1.0f));
//        gl::drawString( "Raw texture fetched from FBO draw target", vec2(10,10) );
//        gl::draw( mBlurDrawTarget->getBluredFbo()->getColorTexture(), ci::Rectf( 0,0, mBlurDrawTarget->getInputSize().x, mBlurDrawTarget->getInputSize().y ) );
    }
}

CINDER_APP( EcsRenderingApp, RendererGl )
