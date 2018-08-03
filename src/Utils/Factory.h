//
//  Factory.h
//  EcsSerialization
//
//  Created by Henrique on 7/9/18.
//

#ifndef Factory_h
#define Factory_h

#include "ecs/Manager.h"
#include "Utils/Transform.h"


#include "cinder/Json.h"
#include "cinder/App/AppBase.h"

namespace ecs {
    namespace factory{
        
        inline void initialize(ecs::Manager& iManager, bool connectUpdate = true, bool connectPostDraw = true ){
            
            iManager.setup();
            
            if( connectUpdate ){
                ci::app::AppBase::get()->getSignalUpdate().connect( [&iManager]{
                    iManager.update();
                } );
            }
            
            if( connectPostDraw ){
                ci::app::AppBase::get()->getWindow()->getSignalPostDraw().connect( [&iManager]{
                    iManager.draw();
                });
            }
        }
        
        
        
        inline void saveComponents( ci::JsonTree* json, ecs::EntityRef entity  ){
        
            for( auto& component : entity->getComponents() ){
                component.lock()->getFactory()->save( json );
            }
            
        }
        inline void saveTree( ci::JsonTree* json, ecs::EntityRef entity, unsigned int depth = 0){
            
            
            auto entityJson = ci::JsonTree::makeObject( std::to_string( entity->getId()) );
            
            auto transformHandle =  entity->getComponent<Transform>().lock();
            
            
            auto children =  transformHandle->getChildren();
            
            saveComponents(&entityJson, entity);

            if( transformHandle->hasParent() ){
                auto parentJson = ci::JsonTree("parent_id",  transformHandle->getParent().lock()->getEntity().lock()->getId() );
                entityJson.addChild( parentJson );
            }

            json->addChild( entityJson );
            
            for( auto& child : children ){
    
                saveTree( json, child.lock()->getEntity().lock()  );
            }
        }
        
        inline void loadComponent( ci::JsonTree* json, ecs::EntityRef entity ){
            auto componentKey =  json->getKey();
            
            auto raw = ecs::Manager::typeFactory[ componentKey ]->create();
            raw->getFactory()->load( json );
            entity->addComponent(raw);
        }
        
        inline ecs::EntityRef loadTree(ci::JsonTree* json, ecs::Manager& iManager){
        
            struct EntityInfo {
                
                EntityInfo() = default;
                EntityInfo(ecs::EntityRef e, std::string& i) : entity(e), parentId(i) { }
                
                ecs::EntityRef entity;
                std::string parentId;
            };
            
            ecs::EntityRef e;
            std::map<std::string, EntityInfo> entityMap;
            
            for( auto& jsonEntity : json->getChildren() ){
                
                 e = iManager.createEntity();
                std::string parentId = "";
                for( auto& jsonComponent : jsonEntity.getChildren() ){
                    
                    if( jsonComponent.getKey() != "parent_id" ){
                        
                        auto j = jsonComponent;
                        ci::app::console() << j << std::endl;
                        loadComponent(&j, e );
                        
                    }else{
                        parentId = jsonComponent.getValue();
                    }
    
                }
                entityMap[ jsonEntity.getKey() ] = EntityInfo(e, parentId);
            }
            
            
            // iterate map and build parent child relationship
            for( auto& infoPair : entityMap ){
                
                const auto& entityInfo = infoPair.second;

                if( entityInfo.parentId == ""  ){
                    e = entityInfo.entity;
                    continue;
                }
                
                EntityRef parent;
                try{
                    parent = entityMap.at( entityInfo.parentId ).entity;
                }catch(std::exception &e){
                    console() << e.what() << endl;
                }
                
                
                if(  parent != nullptr ){
                    auto transformHandle = entityInfo.entity->getComponent<Transform>().lock();
                    transformHandle->setParent( parent->getComponent<Transform>(), false  );
                }
                
                

            }

            return e;
             
        }
        
        
        
    }// eof factory namespace
} //eof ecs namespace

#endif /* Factory_h */
