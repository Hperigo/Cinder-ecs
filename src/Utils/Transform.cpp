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
    
    ci::mat4 transform;
    transform *= glm::translate( localPos + anchorPoint);
    transform *= glm::toMat4( mRotation ); //glm::rotate(localRotation, vec3(0,0,1));
    transform *= glm::scale( localScale );
    transform *= glm::translate( -anchorPoint );
    
    mCTransform = transform;
    if(auto p = parent.lock())
    {
        mWorldCTransform = p->getWorldCTransform() * mCTransform;
    }else{
        mWorldCTransform = mCTransform;
    }
    
    if( emitSignal ){
        auto thisHandle = CTransformHandle(shared_from_this());
        onUpdateSignal.emit( thisHandle );
    }
    
    mNeedsUpdate = false;

    for(auto& c : children){
        c.lock()->updateMatrices();
    }

}

// CTransformation Functions ------

void Transform::setCTransform(const mat4 &transform){
    
    mCTransform = transform;
    if(auto p = parent.lock())
    {
        mWorldCTransform = p->getWorldCTransform() * mCTransform;
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
    
    if(auto p = parent.lock())
    {
        auto newP = glm::inverse(p->getWorldCTransform()) * glm::vec4(pos, 1);
        localPos = newP;
    }else{
        localPos = pos;
    }
    
    localPos -= anchorPoint;
    
    mNeedsUpdate = true;
}

// Scale -------

void Transform::setWorldScale(const vec3& scale){
    
    if(auto p = parent.lock())
    {
        vec3 invScale = (1.0f /  p->getWorldScale() );
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
    
    if(auto p = parent.lock()){
        return p->getWorldScale() * localScale;
    }else{
        return localScale;
    }
}

// Rotation -------
void Transform::setWorldRotation( float radians ){
    
    auto q = glm::angleAxis( radians, ci::vec3( 0, 0, 1 ) );
    
    if(auto p = parent.lock()){
        
        auto invParent = glm::inverse( p->getWorldRotation() );
        mRotation = q * invParent;
        
    }else{
        mRotation = q;
    }
    
    mNeedsUpdate = true;
}

void Transform::setWorldRotation(const glm::quat& q ){
    
    if(auto p = parent.lock()){
        auto invParent = glm::inverse( p->getWorldRotation() );
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

    
    if(auto p = parent.lock()){
        return mRotation * p->getWorldRotation();
    }else{
        return mRotation;
    }
}

void Transform::setParent( CTransformHandle _parent, bool keepWorldCTransform){
    
    if( _parent.lock()->getId() == mId){
        return;
    }
    
    parent = _parent;
    parent.lock()->addChildToList( CTransformHandle( shared_from_this() )  );
    
    if( keepWorldCTransform ){
        setWorldPos( localPos );
        setWorldScale(localScale);
        setWorldRotation(mRotation);
    }
    
    mNeedsUpdate = true;
}


void Transform::removeParent(bool keepWordCTransform, bool removeFromList){

    if( removeFromList ){
        parent.lock()->removeChildFromList( shared_from_this() );
    }
    
    auto p = parent.lock();
    
    if(p && keepWordCTransform){
        
        auto newPos =  mWorldCTransform * vec4(0,0, 0, 1);
        localPos = vec3( newPos.x, newPos.y, newPos.z );
        
        auto newScale =  localScale * p->getWorldScale();
        localScale = vec3( newScale.x, newScale.y, newScale.z );
        
        mRotation = mRotation * p->getWorldRotation();
        
        parent = CTransformHandle();
    }
    
    parent = CTransformHandle();

    mNeedsUpdate = true;
}

CTransformHandle Transform::findChild( CTransformHandle child ){
    
    
    if( child.lock() == nullptr ){
        return CTransformHandle();
    }
    
    auto iter = std::find_if(children.begin(), children.end(), [child](const CTransformHandle & handle) -> bool {
        
        auto ptr = handle.lock();
        
        return (ptr != nullptr) && (ptr == child.lock());
        
    });
    
    if( iter == children.end() ){
        return CTransformHandle();
    }
    
    return *iter;
}



bool Transform::addChild(CTransformHandle transform){

    bool alreadyInTree = transform.lock()->hasChild( shared_from_this() ) || hasChild(transform);
    
    if ( !alreadyInTree ){
        transform.lock()->setParent( shared_from_this() );
        transform.lock()->mNeedsUpdate = true;
        return true;
    }else{
        return false;
    }
}


bool Transform::removeChild( CTransformHandle transform ){
    
    auto findIt = findChild(transform);
    
    if ( findIt.lock() ){
        transform.lock()->removeParent();
        return true;
    }else{
        return false;
    }
}


bool Transform::hasChild(const CTransformHandle  child, bool recursive ){
    
    auto findIt = findChild(child);
    
    if( findIt.lock() ){
        return true;
    }
    
    if( recursive  ){
        for( auto& c : children  ){
            
            if(  c.lock()->hasChild( shared_from_this() ) ){
                return true;
            }
        }
    }
   
    return false;
}


bool Transform::removeChildFromList( CTransformHandle child){
    
    auto findIt = findChild(child);
    
    if( findIt.lock() ){
        
        auto rmFn = [child](const CTransformHandle& t ) -> bool{
            return child.lock() == t.lock();
        };
        
        children.erase(  std::remove_if( children.begin(), children.end(), rmFn ), children.end() );
        
        return true;
    }
    return false;
}

bool Transform::addChildToList( CTransformHandle child){
    
    auto findIt = findChild( child );
    
    if( ! findIt.lock() ){
        children.push_back(child);
        return true;
    }
    
    return false;
}

CTransformHandle Transform::getRoot(){
    
    CTransformHandle current = CTransformHandle( shared_from_this() ) ;
    while( current.lock()->hasParent()  ){
        current = current.lock()->getParent();
    }
    
    return current;
}


void Transform::descendTree(const std::function<void (CTransformHandle &, CTransformHandle &)> &fn){
    
    for( auto &c : children ) {
        auto thisHandle = CTransformHandle(shared_from_this());
        fn(thisHandle,  c);
        c.lock()->descendTree(fn);
    }
    
}

ecs::EntityRef ImGui::DrawTree( std::shared_ptr<Transform> &iTransform){
    
        
    static int selection_mask = (1 << 0);
    int node_clicked = -1;
    
    ecs::EntityRef selectedEntity = nullptr;
    
    
    std::function<void(std::shared_ptr<Transform>&)> drawChildren = [&]( std::shared_ptr<Transform>& root ){
        
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
                    auto ptr = child.lock();
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


