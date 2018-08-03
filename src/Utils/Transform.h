//
//  Transform.h
//  LittleECS
//
//  Created by Henrique on 8/21/17.
//
//

#ifndef CCTransform_h
#define CCTransform_h

#include "Component.h"
#include "ecs/System.h"
#include "cinder/Vector.h"


#include "cinder/Json.h"
#include "CinderImGui.h"

using CTransformHandle = std::weak_ptr<class Transform>;

class Transform : public ecs::Component, public std::enable_shared_from_this<Transform>{

public:
    Transform();
    Transform( const ci::vec3& pos_ );
    Transform( const Transform& other );
    ~Transform();

    void onDestroy() override {
        
        // clean up children
        if( children.size() > 0 ){
            for( auto& c : children ){
                c.lock()->removeParent(false, false);
            }
        }
        
    }
    
    
    void setCTransform(const ci::mat4& transform );
    
    ci::mat4 getCTransformMatrix() const { return mCTransform; }
    ci::mat4 getWorldCTransform() {
        
        if( needsUpdate() ){
            updateMatrices();
        }
        
        return mWorldCTransform;
    }
    
    // Position ------------------------------
    ci::vec3 getWorldPos();
    void setWorldPos(const ci::vec3& pos);
    
    ci::vec3 getPos()  {
        
        if(needsUpdate()){
            updateMatrices(false);
        }
        return localPos;
        
    }
    void setPos(const ci::vec3& pos){ localPos = pos; mNeedsUpdate = true; }
    
    ci::vec3* getPosPtr(){  return &localPos; }
    
    // anchor point -----
    
    void setAnchorPoint(const ci::vec3& p ){  anchorPoint = p; mNeedsUpdate = true; }
    ci::vec3 getAnchorPoint() const { return anchorPoint; }
    ci::vec3* getAnchorPointPtr() { return &anchorPoint; }
    
    // Scale --------------------------------
    
    void setWorldScale(const ci::vec3& scale );
    ci::vec3 getWorldScale();
    
    void setScale(const ci::vec3& scale ){ localScale = scale; mNeedsUpdate = true; }
    void setScale( float s ) { setScale( ci::vec3(s,s,s) ); mNeedsUpdate = true; }
    
    ci::vec3* getScalePtr(){  return &localScale; }
    
    ci::vec3 getScale()  {
        
        if(needsUpdate()){
            updateMatrices(false);
        }
        
        return localScale;
    }
    
    
    // Rotation --------------------------------
    
    void setWorldRotation( float radians );
    void setWorldRotation(const ci::quat& q );
    

    void setRotation( float radians ){
        mRotation = glm::angleAxis( radians, glm::vec3( 0, 0, 1 ) );
        mNeedsUpdate = true;
    }
    
    void setRotation(const glm::quat& rotation ){ mRotation = rotation;  mNeedsUpdate = true; }
    
    
    glm::quat getRotation() {
        
        if(needsUpdate()){
            updateMatrices(false);
        }
        
        return mRotation;
        
    }
    float getRotationRadians()  {
        
        if(needsUpdate()){
            updateMatrices(false);
        }
        
        return glm::eulerAngles(mRotation).z;
    }
    glm::quat getWorldRotation();
    glm::quat* getRotationPtr(){  return &mRotation; }
    
    float getWorldRotationRadians()  { return glm::eulerAngles( getWorldRotation() ).z; }
    
    // Parenting ----
    
    
    void setParent( CTransformHandle _parent, bool keepWordCTransform = true );
    void removeParent(bool keepWorldCTransform = true, bool removeFromList = true);
    
    CTransformHandle getParent() const { return parent; }
    
    std::vector<CTransformHandle> getChildren() const { return children; }
    
    
    bool needsUpdate() {
        bool needs = (mNeedsUpdate || mAlwaysUpdate);
        return needs;
    }

    bool addChild( CTransformHandle transform );
    bool removeChild( CTransformHandle transform );
    
    // todo: rename to containChild
    bool hasChild(const CTransformHandle child,  bool recursive = true );
    bool hasParent() const { return (parent.lock() != nullptr); }
    
    CTransformHandle getRoot();
    
    bool isLeaf()const { return children.size() == 0 ? true : false; }
    
    bool removeChildFromList(CTransformHandle child);
    bool addChildToList(CTransformHandle child);
    
    CTransformHandle findChild( CTransformHandle child );
    
    /// Visit all of this components' descendents depth-first. TODO: separate visitor patterns from data.
    void descendTree(const std::function<void (CTransformHandle &parent, CTransformHandle &child)> &fn);

    
    size_t getId() const { return mId; }
    void setId(size_t i){ mId = i; }

    void updateMatrices(bool emitSignal = true);
    
    void setAlwaysUpdate( bool v ){  mAlwaysUpdate = v; }
    bool getAlwaysUpdate(){ return mAlwaysUpdate; }
    
    
    ci::signals::Signal< void( CTransformHandle& handle ) >& getUpdateSignal(){ return onUpdateSignal; }
    
protected:
    
    bool mNeedsUpdate = true;
    bool mAlwaysUpdate = false;
    
    ci::signals::Signal< void( CTransformHandle& handle ) > onUpdateSignal;
    
    glm::vec3 localPos;
    glm::vec3 anchorPoint;
    
    glm::vec3 rotation;
    glm::vec3 localScale{1.0f, 1.0f, 1.0f};
    
    glm::quat mRotation;
    
    glm::mat4 mCTransform;
    glm::mat4 mWorldCTransform;
    
    std::vector<CTransformHandle> children;
    CTransformHandle parent;
    
    
    static size_t transformId;
    size_t mId = 0;
};



template <>
struct ecs::ComponentFactoryTemplate<Transform> : public ecs::ComponentFactory<Transform> {
    
    ComponentFactoryTemplate(){
        ComponentFactory();
    }

    
    void load(void* archiver) override {

        ci::JsonTree& tree = *static_cast<ci::JsonTree*>( archiver );
        
        ci::vec3 pos;
        pos.x = tree.getChild("pos").getChild(0).getValue<float>();
        pos.y = tree.getChild("pos").getChild(1).getValue<float>();
        pos.z = tree.getChild("pos").getChild(2).getValue<float>();
        
        
        ci::vec3 anchorPoint;
        anchorPoint.x = tree.getChild("anchor").getChild(0).getValue<float>();
        anchorPoint.y = tree.getChild("anchor").getChild(1).getValue<float>();
        anchorPoint.z = tree.getChild("anchor").getChild(2).getValue<float>();
        
        ci::vec3 scale;
        scale.x = tree.getChild("scale").getChild(0).getValue<float>();
        scale.y = tree.getChild("scale").getChild(1).getValue<float>();
        scale.z = tree.getChild("scale").getChild(2).getValue<float>();
        
        
        float r;
        r = tree.getChild("rotation").getChild(0).getValue<float>();
        owner->setPos( pos );
        owner->setAnchorPoint(anchorPoint);
        owner->setScale( scale );
        owner->setRotation( r );
    }
    
    void save(void* archiver) override {
        
        ci::JsonTree* tree = static_cast<ci::JsonTree*>( archiver );
        
        auto tJson = ci::JsonTree::makeArray( std::to_string(_id) );
    
        auto pJson = ci::JsonTree::makeArray("pos");
        pJson.addChild( ci::JsonTree("",  owner->getPos().x) );
        pJson.addChild( ci::JsonTree("",  owner->getPos().y) );
        pJson.addChild( ci::JsonTree("",  owner->getPos().z) );
        
        auto aJson = ci::JsonTree::makeArray("anchor");
        aJson.addChild( ci::JsonTree("",  owner->getAnchorPoint().x) );
        aJson.addChild( ci::JsonTree("",  owner->getAnchorPoint().y) );
        aJson.addChild( ci::JsonTree("",  owner->getAnchorPoint().z) );
        
        auto sJson = ci::JsonTree::makeArray("scale");
        sJson.addChild( ci::JsonTree("",  owner->getScale().x) );
        sJson.addChild( ci::JsonTree("",  owner->getScale().y) );
        sJson.addChild( ci::JsonTree("",  owner->getScale().z) );
        
        auto rJson = ci::JsonTree::makeArray("rotation");
        rJson.addChild( ci::JsonTree("", owner->getRotationRadians()) );
        
        
        tJson.addChild( pJson );
        tJson.addChild( aJson );
        tJson.addChild( rJson );
        tJson.addChild( sJson );
        
        tree->addChild( tJson );
    }
};

//template<>
//inline Transform ecs::ComponentFactory<Transform>::object = Transform();



namespace ImGui{
    
    inline void DrawTransform2D( Transform* t, int i ){
        
        ui::DragFloat3( (std::to_string(i) + " position").c_str(), &(*t->getPosPtr())[0]  );
        ui::DragFloat3( (std::to_string(i) + " scale").c_str(), &(*t->getScalePtr())[0], 0.01f  );
        ui::DragFloat3( (std::to_string(i) + " anchor pont").c_str(), &(*t->getAnchorPointPtr())[0], 0.01f  );
        
        auto radians = t->getRotationRadians();
        if(  ui::DragFloat( (std::to_string(i) + "rotation radians").c_str(), &radians, 0.01f ) ){
            t->setRotation( radians );
        }
        
        t->updateMatrices();
    }
    
    ecs::EntityRef DrawTree( std::shared_ptr<Transform>& root );
    
}


#endif /* CCCTransform_h */
