#include <iostream>
#include <type_traits>
#include <ctype.h>
#include <numeric>
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>
#include <map>
#include <cassert>
#include <cmath>
#include <memory>

#include "Entity.hpp"
#include "EntityArchetype.hpp"
#include "EntityChunk.hpp"
#include "EntityManager.hpp"
#include "EntityGroup.hpp"

struct IComponentData
{};

struct Health {
	Health(float v) {
		Value = v;
	}

	float Value;
};

struct Mortal {};
struct Immortal {};

#define TEST_PRINT_NAME(name) std::cout << "[TEST] " << name << "..............";
#define TEST_PRINT_PASSED std::cout << "passed.\n";
#define TEST_PRINT_FAILED(text) std::cout << "failed. " << text << std::endl;
#define TEST(name, assertion, failureText) TEST_PRINT_NAME(name) \
												 if (assertion) { TEST_PRINT_PASSED } else { TEST_PRINT_FAILED(failureText) }
int main()
{
    std::cout << "Hello World!\n";

	EntityArchetype archetype = CreateArchetype<int, float, char, Health, Mortal>();
	EntityArchetype archetype2 = CreateArchetype<int, float, Health, char, Mortal, Immortal>();
	EntityArchetype archetype3 = CreateArchetype<Health, Mortal, long>();
	std::cout << "Archetypes are : " << (archetype == archetype2 ? "equal\n" : "not equal\n");

	for (unsigned int i = 0; i < archetype.Count; i++) {
		std::cout << archetype.Hashes[i] << "\n" << archetype.Sizes[i] << "\n" << archetype.Names[i] << "\n";
	}

	EntityManager entityManager;

	Entity e1 = entityManager.CreateEntity();
	Entity e2 = entityManager.CreateEntity(archetype);
	Entity e = entityManager.CreateEntity(archetype);

	entityManager.DestroyEntity(e1);
	entityManager.DestroyEntity(e2);

	entityManager.SetComponentData(e, 2);
	entityManager.SetComponentData(e, Health(100));
	std::cout << entityManager.GetComponentData<int>(e) << "\ne index ";

	entityManager.CreateEntity(archetype2);

	auto e3 = entityManager.CreateEntity(archetype3);
	entityManager.SetComponentData(e3, Health(200));

	std::cout << "Exists " << entityManager.Exists(e) << entityManager.Exists(e2) << entityManager.Exists(e1) << std::endl;

	std::cout << "------------ Tests ------------\n\n";
	TEST("Archetype with different component order are the same", archetype == archetype2, "");
	TEST("Existing entity exists", entityManager.Exists(e), "");

	/*
	std::cout << "e2 index in chunk " << entityManager.m_Entities[e2.index].IndexInChunk << "\n";
	std::cout << "e index in chunk " << entityManager.m_Entities[e.index].IndexInChunk << "\n";

	std::cout << "new index " << entityManager.CreateEntity().index << "\n new index ";
	std::cout << entityManager.CreateEntity().index << "\n new index ";
	std::cout << entityManager.CreateEntity().index << "\n new index ";
	std::cout << entityManager.CreateEntity().index << "\n";
	std::cout << "e index in chunk " << entityManager.m_Entities[e.index].IndexInChunk << "\n";
	*/

	// entityManager.HasComponent<X>(entity);
	// entityManager.AddComponent<X>(entity);
	// entityManager.AddComponent(entity, X);
	// entityManager.RemoveComponent<X>(entity);
	//
	EntityGroup group (EntityGroupAccept<Health, Mortal>(), 
						EntityGroupReject<Immortal>());
	
	EntityGroupArray* iter = entityManager.GetEntityGroup(group);
	

	std::cout << "Number of matched entities: " << iter->Length << std::endl;
	for (size i = 0; i < iter->Length; i++) {
		std::cout << "Entity " << i << " health: " << iter->GetComponentData<Health>((u32)i).Value << "\n";
	}

	//YourClass<BaseClass> obj;
}
