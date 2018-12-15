#ifndef ARCHETYPECHUNK_H
#define ARCHETYPECHUNK_H

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

	inline size_t Count() {
		return LastIndex;
	}

	inline size_t Capacity() {
		return TotalEntities;
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
			size stripe = Archetype.Sizes[i];
			memcpy((void*)(base + dstIndex * stripe), (void*)(base + srcIndex * stripe), stripe);
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

#endif