#include "Entity.hpp"

Entity::Entity(u32 index, u32 version)
{
    this->index = index;
    Version = version;
}

Entity::Entity()
{
    index = 0;
    Version = 0;
}

Entity::Entity(const Entity &other)
{
    index = other.index;
    Version = other.Version;
}

bool Entity::operator==(const Entity &other) const
{
    return (index == other.index && Version == other.Version);
}