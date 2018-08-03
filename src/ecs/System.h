//
// Created by Henrique on 5/31/18.
//

#ifndef LEKSAPP_SYSTEM_H
#define LEKSAPP_SYSTEM_H

#include <memory>

#include "Component.h"


namespace ecs{
    using SystemRef = std::shared_ptr<class System>;
    class Manager;

    class System {
    public:

        virtual void setup() {  }

        virtual void update(){  }

        virtual void draw() {   }
       
        void setUpdatable( bool v ){ updatable = v; }
        void setDrawable( bool v ){ drawable = v; }

        bool isUpdatable() { return updatable; }
        bool setDrawable() { return updatable; }

        Manager* getManager() {  return mManager; }
        
    protected:
        friend  Manager;
        Manager* mManager;
        
        bool updatable = true;
        bool drawable = true;

    };

}// endof ecs




#endif //LEKSAPP_SYSTEM_H
