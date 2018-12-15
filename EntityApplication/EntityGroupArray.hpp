#ifndef ENTITYGROUPARRAY_HPP
#define ENTITYGROUPARRAY_HPP

#include <vector>

#include "EntityChunk.hpp"
#include "Types.hpp"

struct EntityGroupArray {
	std::vector<size> Counts;
	std::vector<ArchetypeChunk*> Chunks;

	size Length;

	void AddChunk(size count, ArchetypeChunk* chunk) {
		Length += count;
		Counts.push_back(count);
		Chunks.push_back(chunk);
	}

	template<typename T>
	T GetComponentData(u32 index) {
		u32 chunkIndex = 0;

		while (index > Counts[chunkIndex] - 1) {
			index -= (u32)Counts[chunkIndex];
			chunkIndex++;
		}

		ArchetypeChunk* Chunk = Chunks[chunkIndex];
		EntityArchetype archetype = Chunk->Archetype;
		T* data = (T*)Chunk->GetComponentAddress(index, archetype.GetComponentIndex(typeid(T).hash_code()));
		return *data;
	}
};

#endif