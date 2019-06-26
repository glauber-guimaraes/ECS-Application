#ifndef ENTITY_ARRAY_HPP
#define ENTITY_ARRAY_HPP

#include <vector>

class EntityArray
{
public:
	std::vector<ArchetypeChunk*> m_Chunks;
	std::vector<unsigned int> m_MaxIndex;
	int m_Length;

	EntityArray(std::vector<ArchetypeChunk*>& matchingChunks) {
		int currentIndex = 0;
		for (unsigned int i = 0; i < matchingChunks.size(); i++) {
			ArchetypeChunk* chunk = matchingChunks[i];
			currentIndex += (int)chunk->Count();

			m_Chunks.push_back(chunk);
			m_MaxIndex.push_back(currentIndex - 1);
		}

		m_Length = currentIndex;
	}

	EntityArray(const EntityArray& other) {
		m_Chunks = other.m_Chunks;
		m_MaxIndex = other.m_MaxIndex;

		m_Length = other.m_Length;
	}

	inline int Length() {
		return m_Length;
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

#endif