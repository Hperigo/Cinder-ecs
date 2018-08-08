//
// Created by Henrique on 5/24/18.
//

#ifndef LEKSAPP_MANAGER_H
#define LEKSAPP_MANAGER_H


#include "Entity.h"
#include "System.h"

#include <vector>
#include <array>

#include "Utils/DrawSystem.h"

#include "cinder/app/AppBase.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace  ecs{
    using ManagerRef = std::shared_ptr<class Manager>;

class Manager {


public:
    Manager(){
        mDrawSystem = createSystem<DrawSystem>();
    }
    
    ~Manager(){

    }


    template<typename... Args>
    static ManagerRef create(Args&&... args){
        return std::make_shared<Manager>( std::forward<Args>(args)...  );
    }

    EntityRef createEntity(){
        
        EntityRef e = std::make_shared<Entity>();
        e->mManager = this;
        
        mEntities.emplace_back(e);
        e->setup();
        
        return e;
    }


    template<typename T, typename... Args>
    std::shared_ptr<T> createEntity(Args&&... args){

        std::shared_ptr<T> e = std::make_shared<T>( std::forward<Args>(args)...  );
        e->mManager = this;
        e->mInfo = std::make_shared< EntityHelper<T> >();
        mEntities.emplace_back(e);
        e->setup();
        
        return e;
    }

    template<typename T, typename... TArgs>
    std::shared_ptr<T> createSystem(TArgs&&... _Args) {

        std::shared_ptr<T> rawSystem( new T(std::forward<TArgs>(_Args)... ));

        SystemRef systemPtr{ rawSystem };
        rawSystem->mManager = this;
        
        mSystems.push_back( rawSystem );
        return  rawSystem;
    }

    void removeSystem(  SystemRef iSystem){
        
        auto sys = std::find( mSystems.begin(), mSystems.end(), iSystem );

        if( sys != mSystems.end() ){
            mSystems.erase(sys);
        }
    }

    void setup(){


        if( needsRefresh == true ){
            refresh();
        }

        for(auto& sys  : mSystems){
            sys->setup();
        }
        
        update();

    }


    void update(){

        if( needsRefresh == true ){
            refresh();
        }


        for(auto& sys  : mSystems){
            
            if( sys->updatable ){
                sys->update();
            }
        }
    }

    void draw(){

        for(auto& sys  : mSystems){
            if( sys->drawable ){
                sys->draw();
            }
        }
    }


    void refresh() {

        if( !needsRefresh ){
            return;
        }

        for( std::size_t i = 0; i < mComponents.size(); ++i ){


            auto& componentVector(mComponents[i]);

            // erase components
            for( auto cIt = componentVector.begin(); cIt != componentVector.end(); ) {
                if( (*cIt)->getEntity().expired() || ! (*cIt)->getEntity().lock()->isAlive() ){
                    cIt = componentVector.erase(cIt);
                }else{
                    ++cIt;
                }
            }

        }


        for( auto eIt = mEntities.begin(); eIt != mEntities.end();  ){

            if( ! (*eIt)->isAlive() || (*eIt == nullptr)  )
            {
                eIt = mEntities.erase( eIt );
            }
            
            else
            {
                ++eIt;
            }
        }
        
        needsRefresh = false;
    }

    void addComponent( ComponentID id, ComponentRef component){
        mComponents[id].push_back( component );
    }

    template<typename T>
    std::vector<std::weak_ptr<T>> getComponents(){

        if( needsRefresh ){
            refresh();
        }

        auto cId = getComponentTypeID<T>();

        auto components = mComponents[cId];

        std::vector<std::weak_ptr< T > > vec;
        for( auto& c : components ){
            vec.push_back( std::weak_ptr<T>{ std::static_pointer_cast<T>(c) } );
        }

        return vec;
    }


    template<class T>
    void setBitset(std::bitset<MaxComponents>* bitset, T head)const {
        bitset->set( head, 1 );
    }


    template <class T, class ...Args>
    void  setBitset(std::bitset<MaxComponents>* bitset, T head ,  Args ... args) const {
        setBitset( bitset, head );
        setBitset( bitset, args ... );
    }

    template <class T>
     const std::vector<ComponentRef>& getComponentsArray() {
         
         if( needsRefresh ){
             refresh();
         }
         
        auto _id = getComponentTypeID<T>();
        return mComponents[_id];
    }
    
    template <class ...Args>
    std::vector<std::shared_ptr<Entity>> getEntitiesWithComponents() const {
        
        std::bitset<MaxComponents> bitsetMask;
        setBitset( &bitsetMask, getComponentTypeID<Args>()... );
        std::vector<std::shared_ptr<Entity>> entities;
        for( auto &e : mEntities ){
            
            bool b = ( e->getComponentBitset() | bitsetMask  ) == e->getComponentBitset(); // check if entity has all the bits in the bitset mask
            if( b ){
                 entities.push_back( e );
             }
            
         }
        
        return entities;
    };
    
    EntityRef copyEntity( const EntityRef& iEntity ){
        
        
        EntityRef e;
        
        iEntity->mInfo->copy( iEntity, e );
        
        mEntities.push_back(e);
        
        for(size_t i = 0; i < e->mComponentBitset.size(); ++i){
            
            if(  e->mComponentBitset[i] == true ){
                
                ComponentRef targetComponent;
                auto sourceComponent = e->mComponentArray[i];

                sourceComponent->getFactory()->copyComponent( sourceComponent, targetComponent.get() );
                targetComponent->mEntity = e;
                mComponents[i].push_back(  targetComponent );
                e->mComponentArray[i] = mComponents[i].back().get();
            }
        }
        
        return e;
    }
    
    
    std::vector<EntityRef>& getEntities() {  return mEntities; }
    std::vector<SystemRef>& getSystems() { return mSystems; }
    
    std::shared_ptr<DrawSystem> getDrawSystem(){
        return mDrawSystem;
    }
    
  
    template<typename T>
    static void registerType( std::string nameOverride = "" ){
        
        if( nameOverride == ""){
            nameOverride = std::to_string( getComponentTypeID<T>() );
        }
        typeFactory[ nameOverride ] = std::make_shared<ComponentFactoryTemplate<T>>();
    }
    static std::map< std::string , std::shared_ptr<internal::ComponentFactoryInterface>> typeFactory;

protected:

    bool needsRefresh{false};
    
    std::array< std::vector<ComponentRef>, MaxComponents> mComponents;
    
    std::vector<EntityRef> mEntities;
    std::vector<SystemRef> mSystems;
    
    std::shared_ptr<DrawSystem> mDrawSystem;
    
    friend class Entity;
};


}


#endif //LEKSAPP_MANAGER_H
