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

#include "catch2/catch.hpp"

#include "Entity.hpp"
#include "EntityArchetype.hpp"
#include "EntityChunk.hpp"
#include "EntityManager.hpp"
#include "EntityGroup.hpp"

struct Health {
	float Value;
};

TEST_CASE("Archetype with different component order are the same")
{
    EntityArchetype archetype = CreateArchetype<char, int, float>();
    EntityArchetype archetype2 = CreateArchetype<int, float, char>();
    REQUIRE(archetype == archetype2);
}

TEST_CASE("EntityManager.Exists tests")
{
    EntityManager entityManager;

    auto e = entityManager.CreateEntity();

    SECTION("Created entity exists") {
        REQUIRE(entityManager.Exists(e));
    }

    SECTION("Destroyed entity doesn't exist") {
        entityManager.DestroyEntity(e);
        REQUIRE_FALSE(entityManager.Exists(e));
    }
}

TEST_CASE("EntityManager.HasComponent tests")
{
    EntityManager entityManager;

    auto e = entityManager.CreateEntity(CreateArchetype<int>());

    SECTION("Entity has valid component") {
        REQUIRE(entityManager.HasComponent<int>(e));
    }

    SECTION("Entity doesn't have invalid component") {
        REQUIRE_FALSE(entityManager.HasComponent<char>(e));
    }
}

TEST_CASE("Component value editing")
{
	EntityManager entityManager;

	auto e = entityManager.CreateEntity(CreateArchetype<int>());
	entityManager.SetComponentData<int>(e, 0);

	SECTION("Get/Set combination") {
		int value = entityManager.GetComponentData<int>(e);
		value += 5;

		entityManager.SetComponentData<int>(e, value);

		REQUIRE(entityManager.GetComponentData<int>(e) == 5);
	}

	SECTION("Return by reference") {
		int& i = entityManager.GetComponentData<int>(e);
		i += 5;

		REQUIRE(entityManager.GetComponentData<int>(e) == 5);
	}
}

void foo() {
	EntityManager entityManager;

	EntityArchetype archetype = CreateArchetype<int, float, char, Health>();
	EntityArchetype archetype2 = CreateArchetype<int, float, Health, char>();

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
	EntityArchetype simpleArc = CreateArchetype<int, float>();
	Entity simpleE = entityManager.CreateEntity(simpleArc);
	entityManager.AddComponent<Health>(simpleE, { 5 });
	std::cout << "After adding component : " << entityManager.GetComponentData<Health>(simpleE).Value << "\n";

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
