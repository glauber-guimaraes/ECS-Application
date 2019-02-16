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
	float Value;
};

#define TEST_PRINT_NAME(name) std::cout << "[TEST] " << name << "..............";
#define TEST_PRINT_PASSED std::cout << "passed.\n";
#define TEST_PRINT_FAILED(text) std::cout << "failed. " << text << std::endl;
#define TEST(name, assertion, failureText) TEST_PRINT_NAME(name) \
												 if (assertion) { TEST_PRINT_PASSED } else { TEST_PRINT_FAILED(failureText) }
#define TEST_EQUAL(name, expected, result) TEST_PRINT_NAME(name) \
												if (expected == result) { TEST_PRINT_PASSED } \
												else { std::cout << "Failed. Expected " << expected << " got " << result << std::endl; }

int main()
{
    std::cout << "Hello World!\n";

	EntityArchetype archetype = CreateArchetype<int, float, char, Health>();
	EntityArchetype archetype2 = CreateArchetype<int, float, Health, char>();
	std::cout << "Archetypes are : " << (archetype == archetype2 ? "equal\n" : "not equal\n");
	std::cout << archetype << std::endl;

	for (unsigned int i = 0; i < archetype.Count(); i++) {
		std::cout << archetype.Hashes[i] << "\n" << archetype.Sizes[i] << "\n" << archetype.Names[i] << "\n";
	}

	std::cout << "..." << std::flush;
	EntityManager entityManager;
	std::cout << "..." << std::flush << std::endl;

	Entity e1 = entityManager.CreateEntity();
	Entity e2 = entityManager.CreateEntity(archetype);
	Entity e = entityManager.CreateEntity(archetype);

	entityManager.DestroyEntity(e1);
	entityManager.DestroyEntity(e2);

	entityManager.SetComponentData(e, 2);

	Health h = { 5 };
	entityManager.SetComponentData(e, h);
	entityManager.SetComponentData<Health>(e, { 10 });

	std::cout << entityManager.GetComponentData<int>(e) << "\ne index ";

	std::cout << "Exists " << entityManager.Exists(e) << entityManager.Exists(e2) << entityManager.Exists(e1) << std::endl;

	std::cout << "------------ Tests ------------\n\n";
	TEST("Archetype with different component order are the same", archetype == archetype2, "");
	TEST("Existing entity exists", entityManager.Exists(e), "");

	TEST("Entity has Health component", entityManager.HasComponent<Health>(e), "");
	TEST("Entity doesn't have Double component", entityManager.HasComponent<double>(e) == false, "");

	EntityArchetype simpleArc = CreateArchetype<int, float>();
	Entity simpleE = entityManager.CreateEntity(simpleArc);
	entityManager.AddComponent<Health>(simpleE, { 5 });
	std::cout << "After adding component : " << entityManager.GetComponentData<Health>(simpleE).Value << "\n";

	TEST_EQUAL("ComponentType returns correct hash code", 
			typeid(Health).hash_code(), ComponentType::Create<Health>().Hash);

	EntityGroup group = entityManager.GetEntityGroup(ComponentType::Create<Health>(), 
						ComponentType::Subtractive<double>());
	
	auto iter = group.GetComponentArray<Health>();
	auto entityIter = group.GetEntityArray();
	
	for (unsigned int i = 0; i < iter.m_Chunks.size(); i++) {
		std::cout << iter.m_MaxIndex[i] << std::endl;
		std::cout << iter.m_TypeIndex[i] << std::endl;
		std::cout << "" << std::endl;
	}

	std::cout << "Number of matched entities: " << iter.Length() << std::endl;
	for (int i = 0; i < iter.Length(); i++) {
		std::cout << "Entity " << i << " health: " << iter[i].Value << "\n";
		std::cout << "Entity " << i << " health: " << entityManager.GetComponentData<Health>(entityIter[i]).Value << "\n";
	}

	entityManager.RemoveComponent<Health>(simpleE);
	entityManager.AddComponent<char>(simpleE);
	entityManager.AddComponent<char*>(simpleE);

	group = entityManager.GetEntityGroup(ComponentType::Create<Health>(),
						ComponentType::Subtractive<double>());

	iter = group.GetComponentArray<Health>();
	entityIter = group.GetEntityArray();

	std::cout << "After remove component." << iter.Length() << "\n";
	for (int i = 0; i < iter.Length(); i++) {
		std::cout << "Entity " << i << " health: " << iter[i].Value << "\n";
		std::cout << "Entity " << i << " health: " << entityManager.GetComponentData<Health>(entityIter[i]).Value << "\n";
	}

}
