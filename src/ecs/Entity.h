//
// Created by Henrique on 5/23/18.
//

#ifndef LEKSAPP_ENTITY_H
#define LEKSAPP_ENTITY_H


#include <bitset>
#include <array>
#include "Component.h"

namespace ecs{

    class Manager;
    class System;

    using EntityRef = std::shared_ptr<Entity>;
    
    
    class Entity : public std::enable_shared_from_this<Entity> {

    public:

        Entity( ) {
            mEntityId = mNumOfEntities;
            mNumOfEntities += 1;
        }

        virtual ~Entity(){
            markRefresh();
        }

        bool isAlive() const { return mIsAlive; }
        virtual  void destroy() {

            mIsAlive = false;
            
            for( auto& c : mComponentArray ){
                    c->onDestroy();
            }
            
            markRefresh();
        };
        
        unsigned int getId() {
            return mEntityId;
        }
    
        virtual void setup() { }

        template < typename T>
        bool hasComponent() const{
            return mComponentBitset[ getComponentTypeID<T>() ];
        }

        template<typename T, typename... TArgs>
        std::weak_ptr<WrapperComponent<T>> addComponentWrapper(TArgs&&... _Args) {
            
//            std::shared_ptr<T> rawComponent( new T(std::forward<TArgs>(_Args)... ));
            
            std::shared_ptr<WrapperComponent<T>> rawComponent( new WrapperComponent<T>( T(std::forward<TArgs>(_Args)... )) );
            
            auto cId = getComponentTypeID<WrapperComponent<T>>();
            
            auto rawHelper = std::make_shared< ComponentFactoryTemplate< WrapperComponent<T> > >();
            rawHelper->owner = rawComponent.get();
            rawComponent->mFactory = rawHelper;
            
            addComponentToManager(cId, rawComponent);
            
            return  rawComponent;
            
        }
        
        template<typename T, typename... TArgs>
        T* addComponent(TArgs&&... _Args) {

//            constexpr if( !std::is_base_of<Component, T> ){
//                assert( 0 );
//            }
            
            std::shared_ptr<T> rawComponent( new T(std::forward<TArgs>(_Args)... ));

            auto cId = getComponentTypeID<T>();

            auto rawHelper = std::make_shared< ComponentFactoryTemplate<T> >();
            rawHelper->owner = rawComponent.get();
            rawComponent->mFactory = rawHelper;
            
            addComponentToManager(cId, rawComponent);
            
            return  rawComponent.get();
        }
        
        void addComponent( ComponentRef& rawComponent ){
        
            addComponentToManager(rawComponent->getFactory()->_id, rawComponent);
            
        }
        


        template<typename T>
        void removeComponent(){

            auto componentTypeID = getComponentTypeID<T>();

            mComponentBitset.set(componentTypeID, 0);
            mComponentArray [ componentTypeID ].lock()->mEntity = std::weak_ptr<Entity>();/**/
            mComponentArray [ componentTypeID ].reset();

            markRefresh();
        }

        template<typename T>
        T* getComponent() {
            assert(hasComponent<T>());

            markRefresh();

            return (T*)mComponentArray[getComponentTypeID<T>()];
        }


        inline std::bitset<MaxComponents> getComponentBitset(){ return mComponentBitset; }
        std::shared_ptr<internal::EntityInfoBase> mInfo;
        
        inline std::vector< Component* > getComponents(){
            std::vector< Component* > components;
            
            for( auto& c : mComponentArray ){
                    components.push_back( c );
            }
            
            return components;
        }
        
        Manager* getManager() { return mManager; }
        
    protected:
        
        void addComponentToManager( ComponentID cId, ComponentRef component );
        void markRefresh();
        
        friend class Manager;
        
        
        Manager* mManager;
        bool mIsAlive{ true };
        
        std::bitset<MaxComponents> mComponentBitset;
        std::array< Component* , MaxComponents> mComponentArray;
        
        static unsigned int mNumOfEntities;
        unsigned int mEntityId;
    };

    template<class T>
    struct EntityHelper :  public internal::EntityInfoBase{
        
        void copy( const EntityRef& source, EntityRef& target) override{
            target = std::make_shared<T>(  *std::static_pointer_cast<T>( source ) );
        }
    };
    
    
}

#endif //LEKSAPP_ENTITY_H
