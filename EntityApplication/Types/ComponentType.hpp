#ifndef COMPONENT_TYPE_HPP
#define COMPONENT_TYPE_HPP

#include <string>

enum EntityGroupType {
	Additive,
	Subtractive
};

struct ComponentType
{
public:
	EntityGroupType Type;
	hash Hash;
	std::string Name;
	typesize Size;

	ComponentType(EntityGroupType type, hash h, std::string name, typesize size) {
		Type = type;
		Hash = h;
		Name = name;
		Size = size;
	}

	template<typename type>
	static ComponentType Create() {
		return ComponentType(EntityGroupType::Additive, typeid(type).hash_code(), typeid(type).name(), sizeof(type));
	}

	template<typename type>
	static ComponentType Subtractive() {
		return ComponentType(EntityGroupType::Subtractive, typeid(type).hash_code(), typeid(type).name(), sizeof(type));
	}
};

#endif