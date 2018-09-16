#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Json.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"
#include "ecs/Manager.h"

#include "Utils/Transform.h"
#include "Utils/TransformSystem.h"

#include "CinderImGui.h"

#include "Utils/Factory.h"
#include "Utils/DrawSystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;


struct  ColorComponent : public ecs::Component{
    
    ColorComponent() = default;
    ColorComponent( ci::Color e  ) : _color(e) { }
    
    ci::Color _color;
    
};



template <>
struct ecs::ComponentFactoryTemplate<ColorComponent> : public ecs::ComponentFactory<ColorComponent> {
    
    ComponentFactoryTemplate(){
        ComponentFactory();
        
    }
    
    void load(void* archiver) override {
        
        console() << "color loading..." << std::endl;
        
        ci::JsonTree& tree = *static_cast<ci::JsonTree*>( archiver );
        owner->_color.r = tree["r"].getValue<float>();
        owner->_color.g = tree["g"].getValue<float>();
        owner->_color.b = tree["b"].getValue<float>();
    }
    
    void save(void* archiver) override {
        
        console() << "color s" << std::endl;
        
        ci::JsonTree* tree = static_cast<ci::JsonTree*>( archiver );
        auto c = owner->_color;
        
        auto colors = ci::JsonTree::makeArray( std::to_string(_id) );
        colors.addChild( ci::JsonTree{ "r", c.r});
        colors.addChild( ci::JsonTree{ "g", c.g});
        colors.addChild( ci::JsonTree{ "b", c.b});
        tree->addChild( colors );
    }
};

struct RectComponent : public ecs::Component, public ecs::IDrawable{
    

    RectComponent() : ecs::IDrawable(){

    }

    void setup() override {
        
        auto manager = getManager();
        
        auto drawTarget = manager->getDrawSystem()->getDefaultDrawTarget();
        setDrawTarget( drawTarget );
        
    }
    
    void draw() override{
        
        auto entity = getEntity().lock();
        gl::ScopedModelMatrix m;
        
        auto c = entity->getComponent<Transform>();
        gl::multModelMatrix(c->getWorldTransform());
        
        gl::ScopedColor sc( entity->getComponent<ColorComponent>()->_color );
        gl::drawSolidRect( _r );

    }
    
    ci::Rectf _r;
};


template <>
struct ecs::ComponentFactoryTemplate<RectComponent> : public ecs::ComponentFactory<RectComponent> {
 
    ComponentFactoryTemplate(){
        owner->setDrawTarget(nullptr);
    }
    
    void load(void* archiver) override{
        
        
        ci::JsonTree& tree = *static_cast<ci::JsonTree*>( archiver );

        float x = tree["x1"].getValue<float>();
        float y = tree["y1"].getValue<float>();
        float x2 = tree["x2"].getValue<float>();
        float y2 = tree["y2"].getValue<float>();
        
        owner->_r.set(x, y, x2, y2);
    }
    
    void save(void* archiver ) override{

        ci::JsonTree* tree = static_cast<ci::JsonTree*>( archiver );
        auto r = owner->_r;
        
        auto rectJson = ci::JsonTree::makeArray( std::to_string(_id) );
        rectJson.addChild( ci::JsonTree{ "x1", r.getX1() });
        rectJson.addChild( ci::JsonTree{ "y1", r.getY1() });
        rectJson.addChild( ci::JsonTree{ "x2", r.getX2() });
        rectJson.addChild( ci::JsonTree{ "y2", r.getY2() });
        tree->addChild( rectJson );
    }
};

class EcsSerializationApp : public App {
  public:
	void setup() override;
	void mouseDrag( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    ecs::Manager mManager;
    ecs::EntityRef _activeEntity;
    
    ci::JsonTree mArchive;
    ecs::EntityRef createEntity( ecs::EntityRef& parent );
    
    std::shared_ptr<TransformSystem> tsys;
    
    
    ecs::DrawSystem* mDrawSystem;
        
};

void EcsSerializationApp::setup()
{

    ecs::Manager::registerType<RectComponent>();
    ecs::Manager::registerType<Transform>();
    ecs::Manager::registerType<ColorComponent>();

    
    tsys = mManager.createSystem<TransformSystem>();
    mDrawSystem = mManager.getDrawSystem();

    tsys->setDrawable(false);
    auto e = ecs::EntityRef();
    _activeEntity = createEntity( e );
    
    auto child = createEntity(  _activeEntity );
    auto childB = createEntity(  _activeEntity );
    childB->getComponent<Transform>()->setPos(vec3(0, 200, 0));
    
    ui::initialize( ui::Options().darkTheme() );
    ecs::factory::initialize( mManager, true, false );
}



ecs::EntityRef EcsSerializationApp::createEntity(ecs::EntityRef &parent){
    
    auto e = mManager.createEntity();
    auto transform = e->addComponent<Transform>();
    transform->setWorldPos( vec3( vec2(getWindowCenter()), 0 ) );
    transform->setAnchorPoint(vec3( 50, 50, 0 ));
    auto color = e->addComponent<ColorComponent>();
    color->_color = Color(CM_HSV, randFloat(1), 1.0f, 0.5f);
    

    
    auto r = e->addComponent<RectComponent>();
    auto radius = 100.f;
    r->_r = Rectf( 0,0, radius, radius );
    
    if(  parent ){
        parent->getComponent<Transform>()->addChild( transform );
        transform->setPos( vec3(100, 0, 0) );
    }
    
    return e;
    
}


void EcsSerializationApp::mouseDrag( MouseEvent event )
{
    if(_activeEntity){
        _activeEntity->getComponent<Transform>()->setWorldPos( vec3( event.getPos(), 0 ) );
    }
    
}

void EcsSerializationApp::update()
{
    
}

void EcsSerializationApp::draw()
{
    gl::clear( Color::gray(0.3f) );

    ui::Text( "%f", getAverageFps());
    ui::Text( "%s", ( "number of entities: " +  to_string( mManager.getEntities().size() )).c_str()  );
    ui::Text( "%s", ("number of Transforms: " + to_string( mManager.getComponents<RectComponent>().size() )).c_str() );
    ui::Text( "%s", ("number of Colors: " + to_string( mManager.getComponents<ColorComponent>().size() )).c_str() );
    
    ui::Text( "%s", ("number of drawables: " + to_string( mManager.getComponents<ColorComponent>().size() )).c_str() );
    ui::Dummy({0, 10 });
    
    ui::Text( "Transform tree" );
        int ii = 0;
        _activeEntity = nullptr;
        auto entities = mManager.getEntitiesWithComponents<Transform>();
            for( auto e : entities){
                
                if( ! e->getComponent<Transform>()->hasParent() ){
                    auto ptr =  e->getComponent<Transform>();
                    auto result = ImGui::DrawTree( ptr );
                    if( result ){
                        _activeEntity = result;
                    }
                }
                ii++;
            }
            
        if(  _activeEntity ){
            
            ui::Dummy({0, 10 });
            ui::Text( "Active transform" );
            auto tComponent = _activeEntity->getComponent<Transform>();
            ui::DrawTransform2D( tComponent);
            auto cc = _activeEntity->getComponent<ColorComponent>();
            ui::Dummy({0, 10 });
            ui::ColorEdit3( "Color", &cc->_color[0] );
            ui::Dummy({0, 10 });
        }
    

    if( ui::Button("shuffle Entities") ){
        
        for( auto e : mManager.getEntitiesWithComponents<RectComponent>()  ){
            
            auto ehandle = e;
            auto radius = randFloat( 10, 30 );
            ehandle->getComponent<RectComponent>()->_r = Rectf( -radius, -radius, radius, radius );
            
            e->getComponent<Transform>()->setAnchorPoint( { 0 , 0, 0.f } );
            
            
            auto color = e->getComponent<ColorComponent>();
            color->_color = Color(CM_HSV, randFloat(1), 1.0f, 0.75f);
            
        }
    }

    
    if( ui::Button("Animate children") ){

        _activeEntity->getComponent<Transform>()->descendTree([&](Transform* root, Transform* child ){
            
                auto transform =  child;
                
                auto pos = randVec3() * 300.f;
                auto scale = randFloat(0.2f, 1.2f);
                pos.z = 0.f;
                timeline().applyPtr( transform->getPosPtr(), pos , 1.0f, EaseInOutAtan());
            
            timeline().applyPtr( transform->getScalePtr(), {scale, scale, 1.0f} , 1.0f, EaseInOutAtan());
            
            
        });
        
    }
    
    if( ui::Button("Animate to 0") ){
        
        _activeEntity->getComponent<Transform>()->descendTree([&](Transform* root, Transform* child ){
            
            auto transform =  child;
            
            auto pos = vec3(0);
            auto scale = randFloat(0.2f, 1.2f);
            pos.z = 0.f;
            timeline().applyPtr( transform->getPosPtr(), pos , 1.0f, EaseInOutAtan());
            
            timeline().applyPtr( transform->getScalePtr(), {scale, scale, 1.0f} , 1.0f, EaseInOutAtan());
            
            
        });
        
    }
    
    
    if( ui::Button("destroy entity 0") ){

        auto entity = mManager.getEntities()[ 0 ];
        entity->destroy();
        if( _activeEntity == entity ){
            _activeEntity.reset();
        }
        mManager.refresh();
    }
    
    
    if( ui::Button("clear Entities") ){
        
        for( auto& e : mManager.getEntities()){
            e->destroy();
        }
        _activeEntity.reset();
     
        mManager.refresh();
    }
    
    ui::Dummy({0, 10 });
    if( ui::Button("save") ){
        
        mArchive = ci::JsonTree();
        ecs::factory::saveTree(&mArchive, _activeEntity);
        console() << "----SAVED----" << std::endl;
        console() << mArchive << endl;
    }
    
    if( ui::Button("load") ){
        
        console() << "----LOADING----" << std::endl;
        console() << mArchive << endl;
        
        if( mArchive.getValue() == "null" ){
            console() << "archive is null, save first" << std:: endl;
            return;
        }
        
        if( _activeEntity ){
            auto e = ecs::factory::loadTree(&mArchive, mManager);
            e->getComponent<Transform>()->setParent( _activeEntity->getComponent<Transform>() );
        }else{
            _activeEntity = ecs::factory::loadTree(&mArchive, mManager);
        }
    }
    

    
    static bool doDraw = true;
    
    ui::Checkbox("do draw", &doDraw);
    
    if( doDraw ){
        mManager.draw();
        tsys->draw();
    }
}

CINDER_APP( EcsSerializationApp, RendererGl )
