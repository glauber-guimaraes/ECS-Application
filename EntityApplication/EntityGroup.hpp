#ifndef ENTITYGROUP_HPP
#define ENTITYGROUP_HPP

#include "Types.hpp"

#include <string>
#include <exception>

enum EntityGroupType {
    Additive,
    Subtractive
};

struct ComponentType
{
public:
    EntityGroupType Type;
    hash Hash;

    ComponentType(EntityGroupType type, hash h) {
        Type = type;
        Hash = h;
    }

    template<typename type>
    static ComponentType Create() {
        return ComponentType(EntityGroupType::Additive, typeid(type).hash_code());
    }

    template<typename type>
    static ComponentType Subtractive() {
        return ComponentType(EntityGroupType::Subtractive, typeid(type).hash_code());
    }

};

class EntityArray
{
public:
	std::vector<ArchetypeChunk*> m_Chunks;
	std::vector<int> m_MaxIndex;
	int Length;

	EntityArray() {}

	EntityArray(const EntityArray& other) {
		m_Chunks = other.m_Chunks;
		m_MaxIndex = other.m_MaxIndex;

		Length = other.Length;
	}

	Entity operator[](size_t index) {
		if (index > (*m_MaxIndex.crbegin())) {
			throw new std::exception();
		}

		int i = 0;
		for (; i < (int)m_MaxIndex.size(); i++) {
			if (index <= m_MaxIndex[i]) {
				break;
			}
		}

		ArchetypeChunk* chunk = m_Chunks[i];
		int indexInChunk;
		if (i == 0) {
			indexInChunk = (int)index;
		}
		else {
			indexInChunk = (int)index - m_MaxIndex[i - 1];
		}

		return Entity(chunk->GetEntityAtIndex(indexInChunk), 0);
	}
};

template<typename T>
class ComponentArray
{
public:
    std::vector<ArchetypeChunk*> m_Chunks;
    std::vector<int> m_MaxIndex;
    std::vector<int> m_TypeIndex;
    int Length;

    ComponentArray() {}

    ComponentArray(const ComponentArray& other) {
        m_Chunks = other.m_Chunks;
        m_MaxIndex = other.m_MaxIndex;
        m_TypeIndex = other.m_TypeIndex;

        Length = other.Length;
    }

    T& operator[](size_t index) {
        if (index > (*m_MaxIndex.crbegin())) {
            throw new std::exception();
        }

		int i = 0;
        for (; i < (int)m_MaxIndex.size(); i++) {
            if (index <= m_MaxIndex[i]) {
                break;
            }
        }

        ArchetypeChunk* chunk = m_Chunks[i];
        int typeIndex = m_TypeIndex[i];
        int indexInChunk;
        if (i == 0) {
            indexInChunk = (int)index;
        } else {
            indexInChunk = (int)index - m_MaxIndex[i - 1] - 1;
        }
        
        T* data = (T*)chunk->GetComponentAddress(indexInChunk, typeIndex);
        return *data;
    }
};

class EntityManager;
class EntityGroup
{
public:
    EntityGroup(EntityManager* entityManager, std::initializer_list<ComponentType> types, std::vector<ArchetypeChunk*> matchingArchetypes) : m_Types(types) {
        m_EntityManager = entityManager;
        m_MatchingArchetypes = matchingArchetypes;
    }

    template<typename type>
    ComponentArray<type> GetComponentArray() {
        ComponentArray<type> componentArray;
        int currentIndex = 0;
        for (int i = 0; i < m_MatchingArchetypes.size(); i++) {
            ArchetypeChunk* chunk = m_MatchingArchetypes[i];
            currentIndex += (int)chunk->Count();
            int typeIndex = chunk->Archetype.GetComponentIndex(typeid(type).hash_code());

            componentArray.m_Chunks.push_back(chunk);
            componentArray.m_MaxIndex.push_back(currentIndex - 1);
            componentArray.m_TypeIndex.push_back(typeIndex);
        }

        componentArray.Length = currentIndex;

        return ComponentArray<type>(componentArray);
    }

	EntityArray GetEntityArray() {
		EntityArray entityArray;
		int currentIndex = 0;
		for (int i = 0; i < m_MatchingArchetypes.size(); i++) {
			ArchetypeChunk* chunk = m_MatchingArchetypes[i];
			currentIndex += (int)chunk->Count();

			entityArray.m_Chunks.push_back(chunk);
			entityArray.m_MaxIndex.push_back(currentIndex - 1);
		}

		entityArray.Length = currentIndex;

		return EntityArray(entityArray);
	}

    EntityManager* m_EntityManager;
    std::vector<ComponentType> m_Types;
    std::vector<ArchetypeChunk*> m_MatchingArchetypes;
};

#endif