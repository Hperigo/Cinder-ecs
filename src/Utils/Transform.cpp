//
//  Transform.cpp
//  PhartsGame
//
//  Created by Henrique on 10/10/17.
//

#include "Transform.h"
#include "ecs/Manager.h"

using namespace ci;
using namespace glm;


size_t Transform::transformId = 0;



Transform::Transform(){
    mId = transformId;
    transformId++;
}

Transform::Transform( const vec3& pos_ ) : localPos(pos_){
    
    mId = transformId;
    transformId++;
    

}

Transform::Transform( const Transform& other ){
    
    
    localPos = other.localPos;
    rotation = other.rotation;
    localScale = other.localScale;
    
    
    if( other.hasParent() ){
        this->setParent( other.getParent() );
    }
    updateMatrices();
}

Transform::~Transform(){
    
    
    children.clear();
    
}


void Transform::updateMatrices(bool emitSignal){
    
    // TODO: Cache the matrices transformss
    
    ci::mat4 transform;
    transform *= glm::translate<float>( localPos + anchorPoint);
    transform *= glm::toMat4<float>( mRotation ); //glm::rotate(localRotation, vec3(0,0,1));
    transform *= glm::scale<float>( localScale );
    transform *= glm::translate<float>( -anchorPoint );
    
    mCTransform = transform;
    
    if(parent)
    {
        mWorldCTransform = parent->getWorldCTransform() * mCTransform;
    }else{
        mWorldCTransform = mCTransform;
    }
    
    if( onUpdateSignal && emitSignal ){
        onUpdateSignal->emit( this );
    }
    
    mNeedsUpdate = false;

    for(auto& c : children){
        c->updateMatrices();
    }

}

// CTransformation Functions ------

void Transform::setCTransform(const mat4 &transform){
    
    mCTransform = transform;
    if(parent)
    {
        
        mWorldCTransform = parent->getWorldCTransform() * mCTransform;
        
    }else{
        
        mWorldCTransform = mCTransform;
    }

    mNeedsUpdate = true;
}


// Position -------

vec3 Transform::getWorldPos() {
    
    if( needsUpdate() ){
        updateMatrices(false);
    }

    vec4 p = mWorldCTransform * vec4(anchorPoint, 1);
    return vec3(p.x, p.y, p.z);
}


void Transform::setWorldPos(const vec3& pos){
    
    if(parent)
    {
        auto newP = glm::inverse(parent->getWorldCTransform()) * glm::vec4(pos, 1);
        localPos = newP;
    }else{
        localPos = pos;
    }
    
    localPos -= anchorPoint;
    
    mNeedsUpdate = true;
}

// Scale -------

void Transform::setWorldScale(const vec3& scale){
    
    if(parent)
    {
        vec3 invScale = (1.0f /  parent->getWorldScale() );
        localScale =  localScale *  invScale;
    }
    else
    {
        localScale = scale;
    }

    mNeedsUpdate = true;
}

vec3 Transform::getWorldScale() {

    if( needsUpdate() ){
        updateMatrices(false);
    }
    
    if(parent)
    {
        return parent->getWorldScale() * localScale;
    }else{
        return localScale;
    }
}

// Rotation -------
void Transform::setWorldRotation( float radians ){
    
    auto q = glm::angleAxis( radians, ci::vec3( 0, 0, 1 ) );
    
    if(parent)
    {
   
        auto invParent = glm::inverse( parent->getWorldRotation() );
        mRotation = q * invParent;
        
    }else{
        mRotation = q;
    }
    
    mNeedsUpdate = true;
}

void Transform::setWorldRotation(const glm::quat& q ){
    
    if(parent)
    {
        auto invParent = glm::inverse( parent->getWorldRotation() );
        mRotation = q * invParent;
        
    }else{
        mRotation = q;
    }
    
    mNeedsUpdate = true;
}

glm::quat Transform::getWorldRotation() {
    
    if( needsUpdate() ){
        updateMatrices(false);
    }

    
    if(parent)
    {
        return mRotation * parent->getWorldRotation();
    }else{
        return mRotation;
    }
}

void Transform::setParent( Transform* _parent, bool keepWorldCTransform)
{
    
    if( _parent->getId() == mId){
        return;
    }
    
    parent = _parent;
    parent->addChildToList( this );
    
    if( keepWorldCTransform ){
        setWorldPos( localPos );
        setWorldScale(localScale);
        setWorldRotation(mRotation);
    }
    
    mNeedsUpdate = true;
}


void Transform::removeParent(bool keepWordCTransform, bool removeFromList){

    if( removeFromList ){
        parent->removeChildFromList( this );
    }
    //TODO clenup
    auto p = parent;
    
    if(p && keepWordCTransform){
        
        auto newPos =  mWorldCTransform * vec4(0,0, 0, 1);
        localPos = vec3( newPos.x, newPos.y, newPos.z );
        
        auto newScale =  localScale * p->getWorldScale();
        localScale = vec3( newScale.x, newScale.y, newScale.z );
        
        mRotation = mRotation * p->getWorldRotation();
        
        parent = nullptr;
    }
    
    parent = nullptr;

    mNeedsUpdate = true;
}

Transform* Transform::findChild(const Transform* child ){
    
    
    if( child == nullptr ){
        return nullptr;
    }
    
    auto iter = std::find_if(children.begin(), children.end(), [child](const Transform* handle) -> bool {
        return (handle != nullptr) && (handle == child);
    });
    
    if( iter == children.end() ){
        return nullptr;
    }
    
    return *iter;
}



bool Transform::addChild(Transform* transform){

    bool alreadyInTree = transform->hasChild( this ) || hasChild(transform);
    
    if ( !alreadyInTree ){
        
        transform->setParent( this );
        transform->mNeedsUpdate = true;
        
        return true;
    }else{
        return false;
    }
}


bool Transform::removeChild( Transform* transform ){
    
    auto found = findChild(transform);
    
    if ( found ){
        transform->removeParent();
        return true;
    }else{
        return false;
    }
}


bool Transform::hasChild(const Transform* child, bool recursive ){
    
    auto found = findChild(child);
    
    if( found ){
        return true;
    }
    
    if( recursive  ){
        for( auto& c : children  ){
            
            if(  c->hasChild( this ) ){
                return true;
            }
        }
    }
   
    return false;
}


bool Transform::removeChildFromList( Transform* child){
    
    auto findIt = findChild(child);
    
    if( findIt ){
        
        auto rmFn = [child](const Transform* t ) -> bool{
            return child == t;
        };
        
        children.erase(  std::remove_if( children.begin(), children.end(), rmFn ), children.end() );
        
        return true;
    }
    return false;
}

bool Transform::addChildToList( Transform* child){
    
    auto findIt = findChild( child );
    
    if( ! findIt ){
        children.push_back(child);
        return true;
    }
    
    return false;
}

Transform* Transform::getRoot(){
    
    Transform* current = this ;
    while( current->hasParent()  ){
        current = current->getParent();
    }
    
    return current;
}


void Transform::descendTree(const std::function<void (Transform* &, Transform* &)> &fn){
    
    for( auto &c : children ) {
        auto thisHandle = this;
        fn(thisHandle,  c);
        c->descendTree(fn);
    }
    
}

ecs::EntityRef ImGui::DrawTree(const Transform* iTransform){
    

    static int selection_mask = (1 << 0);
    int node_clicked = -1;
    
    ecs::EntityRef selectedEntity = nullptr;
    
    
    std::function<void(const Transform*)> drawChildren = [&](const Transform* root ){
        
//        if( root == nullptr ){
//            return;
//        }
        
        auto rootId = root->getEntity().lock()->getId();
        auto id_text = "e id: " + std::to_string( rootId );
        
        auto nodeName = std::to_string(rootId).c_str();
        bool isSelected = ((selection_mask & (1 << rootId)));
        
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((isSelected == true) ? ImGuiTreeNodeFlags_Selected : 0 ) ;
        
        if( isSelected ){
            selectedEntity = root->getEntity().lock();
        }
        
        
        if( !root->isLeaf() ){
            
            bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)rootId, node_flags, nodeName, rootId);
            
            if(ImGui::IsItemClicked()){
                node_clicked = rootId;
            }
            
            if( nodeOpen ){
                for( auto &child : root->getChildren() ) {
                    auto ptr = child;
                    drawChildren( ptr );
                }
                ImGui::TreePop();
            }
            
        }else{
            ImGui::TreeNodeEx((void*)(intptr_t)rootId, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen , nodeName, rootId);
            
            if(ImGui::IsItemClicked()){
                node_clicked = rootId;
            }
        }
    }; // end of lambda
    

    drawChildren( iTransform );
    
    if( node_clicked != -1 ){
        selection_mask = (1 << node_clicked);
    }
    
    return  selectedEntity;
}


