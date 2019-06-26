#ifndef COMPONENT_ARRAY_HPP
#define COMPONENT_ARRAY_HPP

#include <vector>

template<typename T>
class ComponentArray
{
public:
    std::vector<ArchetypeChunk*> m_Chunks;
    std::vector<unsigned int> m_MaxIndex;
    std::vector<int> m_TypeIndex;
    int m_Length;

    ComponentArray(std::vector<ArchetypeChunk*>& matchingChunks) {
		int currentIndex = 0;
		hash typeHash = ComponentType::Create<T>().Hash;
		for (unsigned int i = 0; i < matchingChunks.size(); i++) {
			ArchetypeChunk* chunk = matchingChunks[i];
			currentIndex += (int)chunk->Count();
			int typeIndex = chunk->Archetype.GetComponentIndex(typeHash);

			m_Chunks.push_back(chunk);
			m_MaxIndex.push_back(currentIndex - 1);
			m_TypeIndex.push_back(typeIndex);
		}

		m_Length = currentIndex;

	}

    ComponentArray(const ComponentArray& other) {
        m_Chunks = other.m_Chunks;
        m_MaxIndex = other.m_MaxIndex;
        m_TypeIndex = other.m_TypeIndex;

        m_Length = other.m_Length;
    }

	inline int Length() {
		return m_Length;
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

#endif