//
// Created by Henrique on 6/2/18.
//

#ifndef LEKSAPP_UTILS_H
#define LEKSAPP_UTILS_H

#include "Component.h"

#include <list>

namespace ecs{

    class UpdateComponent : public Component {
    public:
        void update();
    };

    class DrawTarget;
    struct IDrawable {
        IDrawable();
        IDrawable( DrawTarget* iDrawTarget );
        virtual ~IDrawable();
        virtual void draw() = 0;
        
        void setDrawTarget( DrawTarget* iDrawTarget );
        
        
        int drawTargetId = -1;
        bool isDirty = false;
        DrawTarget* drawTargetOwner = nullptr;
        
        std::list<IDrawable*>::iterator _listPosition;
    };
}

#endif //LEKSAPP_UTILS_H
