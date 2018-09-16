
## Rationale: 

it's main focus is to *provide a simple api for composing objects and transversing components.* Traditional ecs libs are fine when your have some game like enviroment where most of the objects in the scene can be represented by components and their behaviour by systems. 

But in "creative coding" apps is common to have entities with a very specialized behaviour breaking the ECS mindset, this library tries to fix  that by not having very strict rules, 


It also provides a couple of nice features that I think is usefull on a day-to-day creative coding eviroment like:

1. A transform system and a button system (  mouse click )
2. Draw targets ( usefull for drawing into an FBO's, sccissore'd  scenes,  and post processing ( see ECSRenderingTarget sample )
3. A somewhat naive serialzation mechanism (usufull for saving and loading scenes  )


### Disclameer
 I haven't done any performance tests but it's probably not the fastest ecs in the market, it's also not used in production extensively, it is somewhat of a learning project, so use with care! 
 
 *some references that I used to make this:*
 EntityX
 Vitoreo Romero tutorial

## TODO:

1. improve draw system interface
2. Maybe move components array to raw pointer? ( Today we have an  array of unique_ptr's and another of raw ptrs to that array, that's stupid IMO ) 
3. performance tests?
4. Windows samples are not working
5. the ecs is somewhat framework agnostic, should we do an Openframeworks version?
6. Better  serialization? 
7. Make entities just an integer type? 

### Missing in readme:
1. Serialization
2. registering components
3. loading saving
4. saving tree
5. Transform component
6. Draw Targets


### images:



# Manager, Components & entities


## Managers

A manager is the ‘god like’ class that stores all the entities, components and systems.
We use it to create entities and systems.

You will need to call  ` Manager::setup() `, ` Manager::update() ` and ` Manager::draw() ` in the respective cinder functions

Entities are containers of components 
You can create an entity with the ecs::Manager.

```auto exampleEntity =  mManager.createEntity();```

## Components

```
struct ColorComponent : public ecs::Component{
  Color mColor;
};
```

And adding them is as simple as: 
`exampleEntity->addComponent<ColorComponent>();`

The component will be added to the manager, that way we ensure all ColorComponents live in a vector next to each other. The entity itself only holds  a raw ptr

You can modify a component in a entity by using the function “getComponent<T>()”

```
exampleEntity->getComponent<ColorComponent>()->mColor = Color::gray(0.5);

for( auto& c: mManager.getComponentArray<ColorComponent>() ){

shared_ptr<ColorComponent> cc = static_pointer_cast<ColorComponent>(c);
cc->mColor = Color(1.0f, 0.0f, 0.0f);
}
```
You can also access all entities with a component mask:

```for( auto& c: mManager.getEntitiesWithComponents<ColorComponent, RectComponent>() ){
e.getComponent<ColorComponent>->mColor = Color(1.0f, 0.0f, 0.0f); 
}
```

## Entity inheritance


Image that you have a scene with a lot of buttons, the buttons can be a default entity with components, for example:
```
auto btEntity = mManager.createEntity();
btEntity->addComponent<Button>(); // handles user input, callback etc..
btEntity->addComponent<Transform>();
btEntity->addComponent<Texture>(); // on click call back swaps this texture
```

And a scene entity it self would be an object that is used once and has lot's of specialized behaviours, it's more straight foward to extend the entity class.   

```
struct Scene : public ecs::Entity{

  setup() override {
    add components… transform, etc… 
  }

  animateIn(){
  }

  animateOut(){
  }
  
}

shared_ptr<Scene> scene = mManager.createEntity<Scene>();
```
## Systems

System are the only objects that get update and draw called every frame. They are an efficient way to update a bunch of components together


```
Struct ParticleSystem : ecs::System{

  void update() override{

  }
  void draw() override{

  }
}
```

and to create one: 
```auto particleSystem = mManager.createSystem<ParticleSystem>();```


