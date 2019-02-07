// EntityApplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
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
#include <cxxabi.h>

struct IComponentData
{};

typedef uint32_t u32;
typedef size_t hash;
typedef size_t typesize;
typedef size_t size;

struct Entity
{
public:
	u32 index;

	Entity(u32 index, u32 version) {
		this->index = index;
		m_Version = version;
	}

	Entity() {
		index = 0;
		m_Version = 0;
	}

	bool operator==(const Entity& other) const {
		return (index == other.index && m_Version == other.m_Version);
	}
	
private:
	u32 m_Version;
};

struct EntityArchetype
{
public:
	EntityArchetype() {
	}

	EntityArchetype(u32 typeCount, std::initializer_list<hash> hashes, std::initializer_list<typesize> sizes, std::initializer_list<std::string> names = {})
	{
		Size = std::accumulate(sizes.begin(), sizes.end(), 0);
		Count = typeCount;
		Sizes = sizes;
		Hashes = hashes;
		Names = names;
		// std::copy(sizes.begin(), sizes.end(), Sizes);
		// std::copy(hashes.begin(), hashes.end(), Hashes);
	}

	EntityArchetype(const EntityArchetype& other) {
		Size = other.Size;
		Count = other.Count;
		Sizes = other.Sizes;
		Hashes = other.Hashes;
		Names = other.Names;
	}

	// TODO: Fix EntityArchetypes declared with same types but different order not being equal.
	bool operator==(const EntityArchetype& other) const {
		if (Size != other.Size)
			return false;

		if (Count != other.Count)
			return false;
		
		// We don't need to check sizes and names, only hashes should be enough
		if (std::equal(Hashes.begin(), Hashes.end(), other.Hashes.begin()) == false)
			return false;
		
		return true;
	}

	bool operator<(const EntityArchetype& other) const {
		if (Count < other.Count)
			return true;

		if (Count > other.Count)
			return false;

		if (Size < other.Size)
			return true;

		if (Size > other.Size)
			return false;

		for (u32 i = 0; i < Count; i++) {
			if (Hashes[i] < other.Hashes[i])
				return true;
			if (Hashes[i] > other.Hashes[i])
				return false;
		}

		// They are equal
		return false;
	}

	bool Contains(const EntityArchetype& other) const {
		if (other.Count > this->Count)
			return false;

		for (u32 i = 0; i < other.Count; i++) {
			bool found = false;
			for (u32 j = 0; j < this->Count; j++) {
				if (other.Hashes[i] == this->Hashes[j]) {
					found = true;
					break;
				}
			}

			if (found == false)
				return false;
		}

		return true;
	}

	bool Contains(hash typeHash) const {
		for (u32 i = 0; i < this->Count; i++) {
			if (this->Hashes[i] == typeHash)
				return true;
		}

		return false;
	}

	u32 GetComponentIndex(hash componentHash) {
		int componentIndex = -1;
		for (u32 i = 0; i < Count; i++) {
			if (Hashes[i] == componentHash) {
				componentIndex = i;
				break;
			}
		}

		return componentIndex;
	}

	u32 Size;
	u32 Count;
	std::vector<typesize> Sizes;
	std::vector<hash> Hashes;
	std::vector<std::string> Names;
};

template<typename... Types>
EntityArchetype CreateArchetype() {
	return EntityArchetype(sizeof...(Types), { typeid(Types).hash_code()... }, { sizeof(Types)... }, { typeid(Types).name()... });
}

class ArchetypeChunk
{
public:
	EntityArchetype Archetype;

	void* Data;
	u32 TotalEntities;
	char** Components;

	const size ChunkSize = 0xFFFF;

	u32 LastIndex;

	ArchetypeChunk() {
		Data = malloc(ChunkSize);
	}

	u32 ReserveEntity(u32 entityIndex) {
		u32* EntityDataArray = (u32*)Data;
		EntityDataArray[LastIndex] = entityIndex;
		return LastIndex++;
	}

	u32 RemoveEntity(u32 indexInChunk, bool* moved) {
		// Dont need to move anything if we're removing the last entity of the list.
		bool needDataMove = indexInChunk != (LastIndex - 1);

		if (needDataMove == false) {
			*moved = false;
			LastIndex--;
			return 0;
		}

		// Entity is in the middle of the chunk.
		LastIndex--;
		*moved = true;

		MoveEntityComponentData(LastIndex, indexInChunk, 1);
		u32* EntityDataArray = (u32*)Data;	
		return EntityDataArray[indexInChunk];	
	}

	void MoveEntityComponentData(u32 srcIndex, u32 dstIndex, u32 amount) {
		// Move EntityData
		u32* EntityDataArray = (u32*)Data;

		//EntityDataArray[dstIndex] = EntityDataArray[srcIndex];
		memcpy((void*)(EntityDataArray + dstIndex), (void*)(EntityDataArray + srcIndex), amount * sizeof(u32));

		// Move EntityComponents
		for (u32 i = 0; i < Archetype.Count; i++) {
			char* base = Components[i];
			u32 stripe = Archetype.Sizes[i];
			memcpy((void*)(base + dstIndex*stripe), (void*)(base + srcIndex*stripe), stripe);
		}
	}

	void Alloc(EntityArchetype archetype) {
		this->Archetype = archetype;

		TotalEntities = (int)std::floor((float)ChunkSize / (archetype.Size + sizeof(u32)));
		std::cout << "Allocating space for " << TotalEntities << " entities\n";

		if (Components)
			delete[] Components;
		Components = new char*[archetype.Count];

		char* Pointer = (char*)Data;

		// List of entity indexes comes first.
		Pointer += sizeof(u32) * TotalEntities;

		for (unsigned int i = 0; i < archetype.Count; i++) {
			Components[i] = Pointer;
			Pointer += TotalEntities * archetype.Sizes[i];
		}
	}

	void* GetComponentAddress(u32 entityIndex, u32 componentIndex) {
		char* base = Components[componentIndex];
		return (void*)(base + (entityIndex * Archetype.Sizes[componentIndex]));
	}
};

struct EntityData
{
	u32 IndexInChunk;
	ArchetypeChunk* Chunk;
	u32 Version;
};

class EntityManager
{
public:
	EntityData* m_Entities;
	u32 m_EntitiesFreeIndex;
	std::map<EntityArchetype, ArchetypeChunk*> m_ChunkMap;

	EntityManager () {
		m_Entities = (EntityData*)malloc(sizeof(EntityData) * 1024);
		memset(m_Entities, 0, sizeof(EntityData) * 1024);
		m_EntitiesFreeIndex = 0;
	}

	Entity CreateEntity() {
		u32 newFreeIndex = m_Entities[m_EntitiesFreeIndex].IndexInChunk;
		bool isDeletedEntity = m_Entities[m_EntitiesFreeIndex].Version > 0;

		// This means we never deleted any entities.
		if (isDeletedEntity == false)
			newFreeIndex = m_EntitiesFreeIndex + 1;

		int entityIndex = m_EntitiesFreeIndex;
		int entityVersion = m_Entities[entityIndex].Version;
		m_EntitiesFreeIndex = newFreeIndex;

		return Entity(entityIndex, entityVersion);
	}

	Entity CreateEntity(EntityArchetype Archetype) {
		u32 newFreeIndex = m_Entities[m_EntitiesFreeIndex].IndexInChunk;
		bool isDeletedEntity = m_Entities[m_EntitiesFreeIndex].Version > 0;

		// This means we never deleted any entities.
		if (isDeletedEntity == false)
			newFreeIndex = m_EntitiesFreeIndex + 1;

		int entityIndex = m_EntitiesFreeIndex;
		int entityVersion = m_Entities[entityIndex].Version;
		m_EntitiesFreeIndex = newFreeIndex;

		ArchetypeChunk* chunk = CreateOrGetChunk(Archetype);
		m_Entities[entityIndex].IndexInChunk = chunk->ReserveEntity(entityIndex);
		m_Entities[entityIndex].Chunk = chunk;
		return Entity(entityIndex, entityVersion);
	}

	ArchetypeChunk* CreateOrGetChunk(EntityArchetype archetype) {
		if (m_ChunkMap.find(archetype) == m_ChunkMap.end()) {
			std::cout << "Archetype is not defined, making one.";
			ArchetypeChunk* result = new ArchetypeChunk();
			result->Alloc(archetype);
			m_ChunkMap[archetype] = result;
		}

		return m_ChunkMap[archetype];
	}

	template<typename T>
	void AddComponent(Entity entity, T data) {

	}

	template<typename T>
	void SetComponentData(Entity entity, T data) {
		ArchetypeChunk* Chunk = m_Entities[entity.index].Chunk;
		EntityArchetype archetype = m_Entities[entity.index].Chunk->Archetype;
		T* address = (T*)Chunk->GetComponentAddress(m_Entities[entity.index].IndexInChunk, archetype.GetComponentIndex(typeid(T).hash_code()));
		*address = data;
	}

	template<typename T>
	T GetComponentData(Entity entity) {
		ArchetypeChunk* Chunk = m_Entities[entity.index].Chunk;
		EntityArchetype archetype = m_Entities[entity.index].Chunk->Archetype;
		T* data = (T*)Chunk->GetComponentAddress(m_Entities[entity.index].IndexInChunk, archetype.GetComponentIndex(typeid(T).hash_code()));
		return *data;
	}

	void DestroyEntity(Entity entity) {
		u32 entityIndex = entity.index;
		u32 newFreeIndex = entity.index;
		ArchetypeChunk* chunk = m_Entities[entityIndex].Chunk;
		u32 indexInChunk = m_Entities[entityIndex].IndexInChunk;
		m_Entities[entityIndex].Version++;
		m_Entities[entityIndex].IndexInChunk = m_EntitiesFreeIndex;
		m_Entities[entityIndex].Chunk = 0;

		if (chunk) {
			bool moved;
			u32 movedIndex = chunk->RemoveEntity(indexInChunk, &moved);
			if (moved) {
				m_Entities[movedIndex].IndexInChunk = indexInChunk;
			}
		}
		m_EntitiesFreeIndex = newFreeIndex;
	}
};

struct Health {
	float Value;
};

int main()
{
    std::cout << "Hello World!\n";

	EntityArchetype archetype = CreateArchetype<int, float, char, Health>();
	EntityArchetype archetype2 = CreateArchetype<int, float, Health, char>();
	std::cout << "Archetypes are : " << (archetype == archetype2 ? "equal\n" : "not equal\n");

	for (unsigned int i = 0; i < archetype.Count; i++) {
		std::cout << archetype.Hashes[i] << "\n" << archetype.Sizes[i] << "\n" << archetype.Names[i] << "\n";
	}

	int a = 2;
	float b = 3.15f;
	char c = 'c';

	// TODO(glauber): Add Entity.Version check, probably a == operator overload.
	EntityManager entityManager;
	Entity e1 = entityManager.CreateEntity();
	Entity e2 = entityManager.CreateEntity(archetype);
	Entity e = entityManager.CreateEntity(archetype);
	entityManager.DestroyEntity(e1);
	std::cout << "e2 index in chunk " << entityManager.m_Entities[e2.index].IndexInChunk << "\n";
	std::cout << "e index in chunk " << entityManager.m_Entities[e.index].IndexInChunk << "\n";
	entityManager.DestroyEntity(e2);
	entityManager.SetComponentData(e, a);
	std::cout << entityManager.GetComponentData<int>(e) << "\ne index ";
	std::cout << e.index << "\n new index " << entityManager.CreateEntity().index << "\n new index ";
	std::cout << entityManager.CreateEntity().index << "\n new index ";
	std::cout << entityManager.CreateEntity().index << "\n new index ";
	std::cout << entityManager.CreateEntity().index << "\n";
	std::cout << "e index in chunk " << entityManager.m_Entities[e.index].IndexInChunk << "\n";
	// entityManager.Exists(entity);
	// entityManager.HasComponent<X>(entity);
	// entityManager.AddComponent<X>(entity);
	// entityManager.AddComponent(entity, X);
	// entityManager.RemoveComponent<X>(entity);
	//
	/*
	EntityGroup group (EntityGroupAccept<Health, Mortal>(), 
					   EntityGroupReject<Immortal>());

	EntityIterator iter = entityManager.GetEntityGroup(group);
	std::cout << "Number of matched entities: " << iter.Length;
	for (u32 i = 0; i < iter.Length; i++) {
		std::cout << "Entity " << i << " health: " << iter.GetComponentData<Health>(i).Value << "\n";
	}
	*/

	//YourClass<BaseClass> obj;
}
