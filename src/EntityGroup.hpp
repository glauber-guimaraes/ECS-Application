#ifndef ENTITYGROUP_HPP
#define ENTITYGROUP_HPP

#include "Types.hpp"
#include "Types/ComponentType.hpp"

#include "Iterators/EntityArray.hpp"
#include "Iterators/ComponentArray.hpp"

#include <string>
#include <exception>

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
        return ComponentArray<type>(m_MatchingArchetypes);
    }

	EntityArray GetEntityArray() {
		return EntityArray(m_MatchingArchetypes);
	}

    EntityManager* m_EntityManager;
    std::vector<ComponentType> m_Types;
    std::vector<ArchetypeChunk*> m_MatchingArchetypes;
};

#endif