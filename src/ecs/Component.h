//
// Created by Henrique on 5/23/18.
//

#include <memory>
#include <bitset>

#ifndef ECSSAPP_COMPONENT_H
#define ECSSAPP_COMPONENT_H

namespace ecs{

    // forward decls...
    class Component;

    class Entity;
    using EntityRef = std::shared_ptr<Entity>;
    using EntityHandle = std::weak_ptr<Entity>;
    class Manager;

    using ComponentID = std::size_t;
    using ComponentRef = std::shared_ptr<Component>;

    constexpr std::size_t MaxComponents{120};
    using ComponentBitset = std::bitset<MaxComponents>;
    
    namespace internal{

        static ComponentID lastID{0};
        inline ComponentID getUniqueComponentID() noexcept {
            return lastID++;
        }
    
        struct ComponentFactoryInterface : public std::enable_shared_from_this<ComponentFactoryInterface> {
            virtual void copyComponent(const Component* source, Component* target){};
            virtual void load(void* archiver){};
            virtual void save(void* archiver){};
            virtual ComponentRef create() = 0;
            
            ComponentID _id;
        };
        
        
        struct EntityInfoBase{
            virtual void copy(const EntityRef& source, EntityRef& target) = 0;
        };
        
        template <typename T>
        inline ComponentID getComponentTypeID() noexcept {
            
            static ComponentID typeID { internal::getUniqueComponentID() };
            return typeID;
            
        }
    }
    




    struct Component {

    public:
        virtual void setup() { };
        virtual void onDestroy(){  };


        std::weak_ptr<Entity> getEntity(){ return mEntity; }
        std::weak_ptr<Entity> getEntity() const { return mEntity; }
        Manager* getManager(){ return mManager; }


        std::shared_ptr<internal::ComponentFactoryInterface> getFactory() { return mFactory; }
        void setFactory(const std::shared_ptr<internal::ComponentFactoryInterface>& iFactory ){ mFactory = iFactory; }
    protected:
    
        std::shared_ptr<internal::ComponentFactoryInterface> mFactory;
        
        std::weak_ptr<Entity> mEntity;
        Manager* mManager;
    
        std::size_t mComponentId;

        friend class Entity;
        friend class Manager;
    };
    

    template<typename T>
    struct WrapperComponent : public Component{
        
        WrapperComponent(){
            object = T();
        }
        
        WrapperComponent(const T& input ) : object(input) {
            object = input;
        }
        
        T object;
    };
    
    
    template <typename T>
    inline ComponentID getComponentTypeID() noexcept {
        
        if constexpr ( std::is_base_of<Component, T>::value == true ){
            return internal::getComponentTypeID<T>();
        }else{
            return internal::getComponentTypeID< WrapperComponent<T> >();
        }
    }
    
    
    template<class T>
    struct ComponentFactory :  public internal::ComponentFactoryInterface{
        
            ComponentFactory() {
                owner = &object;
                _id = getComponentTypeID<T>();
            }
            
        void copyComponent(const Component* source, Component* target) override{
//            *source = T( *((T*)source) ); // cast
            
            T object = *((T*)source);
            T* sourceObj = (T*)source;
            
            *sourceObj = *((T*)source);
        }
            
        
        ComponentRef create() override;
            
            
        void save(void* archiver) override{ }
            
        void load(void* archiver) override { }
        
        T* owner;
        static T object;
    };
    
    template<typename T>
    T ecs::ComponentFactory<T>::object = T();

    
    template<class T>
    struct ComponentFactoryTemplate : public ComponentFactory<T>{

    };
    
    template<class T>
    ComponentRef ComponentFactory<T>::create(){
        auto t = std::make_shared<T>();
        
        auto helper = std::make_shared<ComponentFactoryTemplate<T> >();
        helper->owner = t.get();
        t->setFactory(helper);
        
        return t;
    }
    
}//end of namespace



#endif //ECSSAPP_COMPONENT_H
