#ifndef ENTITYARCHETYPE_H
#define ENTITYARCHETYPE_H

#include <algorithm>

struct EntityArchetype
{
public:
	size Size;
	u32 Count;
	std::vector<typesize> Sizes;
	std::vector<hash> Hashes;
	std::vector<std::string> Names;

	EntityArchetype() {
		Size = 0;
		Count = 0;
	}

	EntityArchetype(u32 typeCount, std::initializer_list<hash> hashes, std::initializer_list<typesize> sizes, std::initializer_list<std::string> names = {})
	{
		Size = std::accumulate(sizes.begin(), sizes.end(), (size)0);
		Count = typeCount;
		Sizes = sizes;
		Hashes = hashes;
		Names = names;
		// std::copy(sizes.begin(), sizes.end(), Sizes);
		// std::copy(hashes.begin(), hashes.end(), Hashes);
		SortTypesByHash();
	}

	EntityArchetype(const EntityArchetype& other) {
		Size = other.Size;
		Count = other.Count;
		Sizes = other.Sizes;
		Hashes = other.Hashes;
		Names = other.Names;
	}

	void SortTypesByHash() {
		if (Count == 0)
			return;

		u32 currIndex = 0;
		u32 smallerIndex = 0;

		while (currIndex < Count) {			
			// Find the smallest hash with higher index.
			for (u32 i = currIndex + 1; i < Count; i++) {
				if (Hashes[smallerIndex] > Hashes[i])
					smallerIndex = i;
			}

			// Current position is not the smaller one.
			if (smallerIndex > currIndex) {
				std::swap(Sizes[currIndex], Sizes[smallerIndex]);
				std::swap(Hashes[currIndex], Hashes[smallerIndex]);
				std::swap(Names[currIndex], Names[smallerIndex]);
			}

			currIndex++;
			smallerIndex = currIndex;
		}		
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

	bool ContainsAll(const EntityArchetype& other) const {
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

	bool ContainsAny(const EntityArchetype& other) const {
		for (u32 i = 0; i < other.Count; i++) {
			for (u32 j = 0; j < this->Count; j++) {
				if (other.Hashes[i] == this->Hashes[j]) {
					return true;
				}
			}
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
};

template<typename... Types>
EntityArchetype CreateArchetype() {
	return EntityArchetype(sizeof...(Types), { typeid(Types).hash_code()... }, { sizeof(Types)... }, { typeid(Types).name()... });
}

#endif