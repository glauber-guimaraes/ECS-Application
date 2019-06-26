#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Types.hpp"

struct Entity
{
public:
	u32 index;
	u32 Version;

	Entity();
	Entity(u32 index, u32 version);
	Entity(const Entity& other);

	bool operator==(const Entity& other) const;

private:
};

#endif