#ifndef COMPONENT_TYPE_HPP
#define COMPONENT_TYPE_HPP

#include <string>

enum ComponentAccessMode {
	ReadWrite,
	ReadOnly,
	Subtractive
};

struct ComponentType
{
public:
	ComponentAccessMode Type;
	hash Hash;
	std::string Name;
	typesize Size;

	ComponentType(ComponentAccessMode type, hash h, std::string name, typesize size) {
		Type = type;
		Hash = h;
		Name = name;
		Size = size;
	}

	template<typename type>
	static ComponentType Create() {
		return ComponentType(ComponentAccessMode::ReadWrite, typeid(type).hash_code(), typeid(type).name(), sizeof(type));
	}

	template<typename type>
	static ComponentType Subtractive() {
		return ComponentType(ComponentAccessMode::Subtractive, typeid(type).hash_code(), typeid(type).name(), sizeof(type));
	}
};

#endif