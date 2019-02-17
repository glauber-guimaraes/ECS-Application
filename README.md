[![Build Status](https://travis-ci.com/glauber-guimaraes/ECS-Application.svg?branch=master)](https://travis-ci.com/glauber-guimaraes/ECS-Application)
# ECS-Application
Similar to Unity's ECS in C++ for fun and science

## Usage
### Simple operations
```cpp
// Define a structure for your data. Primitive types can also be used.
struct Health {
    float Value;
};

// Create an archetype.
EntityArchetype archetype = CreateArchetype<Health>();

// Create an entity manager.
EntityManager entityManager;

// Create an entity based off of the archetype.
Entity e = entityManager.CreateEntity(archetype);

// Set data for the entity.
Health h = { 5 };
entityManager.SetComponentData(e, h);
entityManager.SetComponentData<Health>(e, { 10 });

// Get data for the entity.
std::cout << entityManager.GetComponentData<Health>(e).Value;

// Check if entity still exists.
bool exists = entityManager.Exists(e);

// Check if it has a component.
bool has = entityManager.HasComponent<Health>(e);

// Add components at run time.
entityManager.AddComponent<float>(e);
entityManager.AddComponent<int>(e, 27);

// Remove components at run time.
entityManager.RemoveComponent<float>(e);

// Destroy the entity.
entityManager.DestroyEntity(e);
```

### Iterating over entities
```cpp
// Create an entity group.
EntityGroup group = entityManager.GetEntityGroup(
    ComponentType::Create<Health>(), // Matches archetypes with a Health component.
    ComponentType::Subtractive<double>() // Matches archetypes without a double component.
);

// Get an iterator.
auto iter = group.GetComponentArray<Health>(); // Iterate directly over the component.
auto entityIter = group.GetEntityArray(); // Iterate over the entities.

// Loop over the matching entities.
// Element i on both iterators represent the same entity.
for (int i = 0; i < iter.Length(); i++) {
    std::cout << "Entity " << i << " health: " << iter[i].Value << "\n";
    std::cout << "Entity " << i << " health: " << entityManager.GetComponentData<Health>(entityIter[i]).Value << "\n";
}
```
