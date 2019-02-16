#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "EntityGroup.hpp"

#include <vector>

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

	ArchetypeChunk* CreateOrGetChunk(EntityArchetype archetype) {
		if (archetype.Count() == 0)
			return NULL;

		if (m_ChunkMap.find(archetype) == m_ChunkMap.end()) {
			std::cout << "Archetype (" << archetype << ") is not defined, making one.";
			ArchetypeChunk* result = new ArchetypeChunk();
			result->Alloc(archetype);
			m_ChunkMap[archetype] = result;
		}

		return m_ChunkMap[archetype];
	}

	template<typename T, typename std::enable_if<std::is_default_constructible<T>::value, int>::type = 0>
	void AddComponent(Entity entity) {
		AddComponent(entity, T());
	}

	template<typename T>
	void AddComponent(Entity entity, T data) {
		EntityData entityData = m_Entities[entity.index];
		ArchetypeChunk* oldChunk = entityData.Chunk;
		int oldIndex = entityData.IndexInChunk;

		EntityArchetype newArchetype(oldChunk->Archetype);
		newArchetype.AddType(typeid(T).hash_code(), sizeof(T), typeid(T).name());

		ArchetypeChunk* newChunk = CreateOrGetChunk(newArchetype);
		int newIndex = newChunk->ReserveEntity(entity.index);

		MoveEntityData(oldChunk, oldIndex, newChunk, newIndex);
		m_Entities[entity.index].Chunk = newChunk;
		m_Entities[entity.index].IndexInChunk = newIndex;

		SetComponentData(entity, data);

		bool moved;
		u32 movedIndex = oldChunk->RemoveEntity(oldIndex, &moved);
		if (moved) {
			m_Entities[movedIndex].IndexInChunk = oldIndex;
		}
	}

	template<typename T>
	void RemoveComponent(Entity entity) {
		EntityData entityData = m_Entities[entity.index];
		ArchetypeChunk* oldChunk = entityData.Chunk;
		int oldIndex = entityData.IndexInChunk;

		if (oldChunk->Archetype.GetComponentIndex(typeid(T).hash_code()) == -1)
			return;

		EntityArchetype newArchetype(oldChunk->Archetype);
		newArchetype.RemoveType<T>();

		ArchetypeChunk* newChunk = CreateOrGetChunk(newArchetype);
		int newIndex = newChunk->ReserveEntity(entity.index);

		MoveEntityData(oldChunk, oldIndex, newChunk, newIndex);
		m_Entities[entity.index].Chunk = newChunk;
		m_Entities[entity.index].IndexInChunk = newIndex;

		bool moved;
		u32 movedIndex = oldChunk->RemoveEntity(oldIndex, &moved);
		if (moved) {
			m_Entities[movedIndex].IndexInChunk = oldIndex;
		}
	}

	void MoveEntityData(ArchetypeChunk* srcChunk, int srcIndex, ArchetypeChunk* dstChunk, int dstIndex) {
		EntityArchetype srcArchetype = srcChunk->Archetype;
		EntityArchetype dstArchetype = dstChunk->Archetype;

		for (unsigned int i = 0; i < dstArchetype.Count(); i++) {
			hash typeHash = dstArchetype.Hashes[i];

			int srcTypeIndex = srcArchetype.GetComponentIndex(typeHash);
			if (srcTypeIndex == -1)
				continue;

			size_t typeSize = dstArchetype.Sizes[i];

			void* dst = srcChunk->GetComponentAddress(srcIndex, srcTypeIndex);
			void* src = dstChunk->GetComponentAddress(dstIndex, i);
			memcpy(dst, src, typeSize);
		}
	}

	template<typename T>
	bool HasComponent(Entity entity) {
		if (m_Entities[entity.index].Version != entity.Version)
			return false;

		return m_Entities[entity.index].Chunk->Archetype.Contains(typeid(T).hash_code());
	}

	template<typename T>
	void SetComponentData(Entity entity, T data) {
		ArchetypeChunk* Chunk = m_Entities[entity.index].Chunk;
		EntityArchetype archetype = m_Entities[entity.index].Chunk->Archetype;
		T* address = (T*)Chunk->GetComponentAddress(m_Entities[entity.index].IndexInChunk, archetype.GetComponentIndex(typeid(T).hash_code()));
		*address = data;
	}

	// TODO: Check entity version to verify this entity reference is up-to-date.
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

	template<typename...T>
	EntityGroup GetEntityGroup(T... componentTypes) {
		return EntityGroup(this, { componentTypes... }, GetArchetypeChunks({ componentTypes... }));
	}

	std::vector<ArchetypeChunk*> GetArchetypeChunks(std::vector<ComponentType> typeFilters) {
		std::vector<ArchetypeChunk*> chunks;
		for (auto it = m_ChunkMap.begin(); it != m_ChunkMap.end(); it++) {
			bool failed = false;

			// Don't add empty chunks.
			if (it->second->Count() == 0)
				continue;

			for (auto type : typeFilters) {
				bool contains = it->first.Contains(type.Hash);
				bool shouldContain = type.Type == EntityGroupType::Additive;
				if (contains != shouldContain) {
					failed = true;
					break;
				}
			}

			if (failed == true)
				continue;

			chunks.push_back(it->second);
		}

		return chunks;
	}
};

#endif
