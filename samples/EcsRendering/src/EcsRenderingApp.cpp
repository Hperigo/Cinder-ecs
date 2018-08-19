#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"

#include "Utils/Factory.h"

// add a particle system in the Fbo target, it needs to be set as not "drawable" other wise the system will be drawn twice, in the fbo and the default draw call in the manager
#include "Particles.h"

using namespace ci;
using namespace ci::app;
using namespace std;


// Some basic components

struct  ColorComponent : public ecs::Component{
    
    ColorComponent() = default;
    ColorComponent( ci::Color e  ) : _color(e) { }
    
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


        { // draw the generated texture
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


// simple FBo draw target, everything in it's mDrawables vector will be drawn into it's target fbo. The Draw target itself needs to be added in the manager default draw system
struct FboDrawTarget : public ecs::DrawTarget{

    FboDrawTarget(){
        targetFbo = gl::Fbo::create(fboSize.x, fboSize.y);
    }
    
    
    void update() override {
        /* we can use the update method to do some type of z-sorting, etc... */
    }
    
    
    void draw() override {
        
        {
        
            targetFbo->bindFramebuffer();

            gl::clear( ci::Color(1,0,0) );
            gl::ScopedMatrices sm;
            gl::ScopedViewport sv( fboSize );
            gl::setMatricesWindow(fboSize);

            // set matrices, bind FBO etc...
            for(auto& d : mDrawables){
                d->draw();
            }
            
            targetFbo->unbindFramebuffer();
            
        }
        
    }

    vec2 fboSize {400,400};
    gl::FboRef targetFbo;
};


class MyCustomEntity : public ecs::Entity, public  ecs::IDrawable {
    
public:
    MyCustomEntity( float radius ) : mRadius( radius ){   }
    
    
    void setup() override {
        
        App::get()->getSignalUpdate().connect( std::bind( &MyCustomEntity::customUpdateCall, this) );
        
        auto dt = getManager()->getDrawSystem()->getDefaultDrawTarget();
        setDrawTarget( dt.get() );
        
        addComponent<Transform>();
    }
    
    
    void customUpdateCall() {
        mRadius = 10 * (sin(getElapsedFrames() * 0.1) + 2 * 0.5f);
    }
    
    
    void draw() override {
        
        gl::ScopedModelMatrix m;
        
        auto t = getComponent<Transform>();
        
//        gl::multModelMatrix( t->getTransformMatrix() );
        gl::color( Color(0.4, 0.4, 1.0) );
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
    
    ecs::EntityRef mEntity;
    ecs::EntityRef mEntityB;
    std::shared_ptr<MyCustomEntity> mCustom;
    
    std::shared_ptr<FboDrawTarget> mFboDrawTarget;
    std::shared_ptr<ecs::DrawTarget> mDefaultDrawTarget;
    
    std::shared_ptr<ParticleSystem> mParticleSystem;
};

void EcsRenderingApp::setup()
{

    // create and add the FBO draw target to the default manager draw system. The draw system is just a collection of draw targets, you can have a Fbo draw target, scissor draw target or pass-trought
    mFboDrawTarget = std::make_shared<FboDrawTarget>();
    mManager.getDrawSystem()->addDrawTarget( mFboDrawTarget );

    
    auto e = mManager.createEntity();
    e->addComponent<RectComponent>();
    e->removeComponent<RectComponent>();
    e->destroy();
    
    
    { // FBO target ------
        
        // add a particle system to the draw target
        mParticleSystem = mManager.createSystem<ParticleSystem>();
        mParticleSystem->setDrawTarget( mFboDrawTarget.get() );
        
        // create a rotating rect for fun
        mEntity = mManager.createEntity();
        mEntity->addComponent<Transform>()->setPos(vec3(200,200,0));
        mEntity->addComponent<RectComponent>( mFboDrawTarget );
        mEntity->addComponentWrapper<ci::Color>( 0.0f, 0.0f, 1.0f );

        mDefaultDrawTarget = std::make_shared<ecs::DrawTarget>();
        mManager.getDrawSystem()->addDrawTarget( mDefaultDrawTarget );

    }

    
    
    // create a second entity that will draw the result texture from the FboDrawTarget
    {
        mEntityB = mManager.createEntity();
        mEntityB->addComponent<Transform>()->setPos(vec3(getWindowCenter(),0));
        
        mEntityB->addComponent<TextureComponent>(mDefaultDrawTarget);
        mEntityB->getComponent<TextureComponent>()->setDrawTarget(nullptr);
        mEntityB->getComponent<TextureComponent>()->mTexture = mFboDrawTarget->targetFbo->getColorTexture();
        
        mCustom = mManager.createEntity<MyCustomEntity>(100);
    }
    
    ecs::factory::initialize(mManager);
    ui::initialize();
    
    console() << "initialized!" << "💊" << std::endl;
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
    
    
    if(  ui::Button("toogle draw target") ){
        
        
        if(  mEntityB->getComponent<TextureComponent>()->drawTargetOwner == nullptr ){
            mEntityB->getComponent<TextureComponent>()->setDrawTarget(mDefaultDrawTarget.get());
        }else{
            mEntityB->getComponent<TextureComponent>()->setDrawTarget( nullptr );
        }
        
    }
}

void EcsRenderingApp::draw()
{

    gl::clear( Color::gray(0.1f) );
    gl::ScopedViewport sv( getWindowSize() );
    gl::setMatricesWindow( getWindowSize() );
    
    mManager.draw();

    {
        gl::color(ColorA(1.0f,1.0f,1.0f, 1.0f));
        gl::drawString( "Raw texture fetched from FBO draw target", vec2(10,10) );
        gl::draw( mFboDrawTarget->targetFbo->getColorTexture() );
    }
  
}

CINDER_APP( EcsRenderingApp, RendererGl )
