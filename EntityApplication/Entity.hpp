#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Types.hpp"

struct Entity
{
public:
	u32 index;
	u32 Version;

	Entity(u32 index, u32 version) {
		this->index = index;
		Version = version;
	}

	Entity() {
		index = 0;
		Version = 0;
	}

	bool operator==(const Entity& other) const {
		return (index == other.index && Version == other.Version);
	}

private:
};

#endif