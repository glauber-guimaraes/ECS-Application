#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "EntityGroup.hpp"
#include "EntityGroupArray.hpp"

class EntityManager
{
public:
	struct EntityData
	{
		u32 IndexInChunk;
		ArchetypeChunk* Chunk;
		u32 Version;
	};

	EntityData* m_Entities;
	u32 m_EntitiesFreeIndex;
	std::map<EntityArchetype, ArchetypeChunk*> m_ChunkMap;

	EntityManager() {
		m_Entities = (EntityData*)malloc(sizeof(EntityData) * 1024);
		memset(m_Entities, 0, sizeof(EntityData) * 1024);
		m_EntitiesFreeIndex = 0;
	}

	Entity CreateEntity() {
		return CreateEntity(EntityArchetype());
	}

	Entity CreateEntity(EntityArchetype Archetype) {
		u32 newFreeIndex = m_Entities[m_EntitiesFreeIndex].IndexInChunk;
		bool isDeletedEntity = m_Entities[m_EntitiesFreeIndex].Version > 0;

		// This means we never deleted any entities.
		if (isDeletedEntity == false)
			newFreeIndex = m_EntitiesFreeIndex + 1;

		int entityIndex = m_EntitiesFreeIndex;
		// Version = 0 is reserved for uninitialized entities.
		int entityVersion = m_Entities[entityIndex].Version > 0 ? m_Entities[entityIndex].Version : 1;
		m_EntitiesFreeIndex = newFreeIndex;

		ArchetypeChunk* chunk = CreateOrGetChunk(Archetype);

		if (chunk) {
			m_Entities[entityIndex].IndexInChunk = chunk->ReserveEntity(entityIndex);
			m_Entities[entityIndex].Chunk = chunk;
		}

		m_Entities[entityIndex].Version = entityVersion;

		return Entity(entityIndex, entityVersion);
	}

	bool Exists(Entity entity) {
		return m_Entities[entity.index].Version == entity.Version && m_Entities[entity.index].Version != 0;
	}

	EntityGroupArray* GetEntityGroup(EntityGroup& group) {
		EntityGroupArray* result = new EntityGroupArray();

		for (auto it = m_ChunkMap.begin(); it != m_ChunkMap.end(); it++) {
			EntityArchetype arch = it->first;
			ArchetypeChunk* chunk = it->second;

			if (arch.ContainsAll(group.accept) && arch.ContainsAny(group.reject) == false) {
				result->AddChunk(chunk->Count(), chunk);
			}
		}

		return result;
	}

	ArchetypeChunk* CreateOrGetChunk(EntityArchetype archetype) {
		if (archetype.Count == 0)
			return NULL;

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
	void RemoveComponent() {

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
		// This entity has already been destroyed.
		if (entity.Version != m_Entities[entity.index].Version)
			return;

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

#endif