#ifndef ENTITYGROUP_HPP
#define ENTITYGROUP_HPP

#include "EntityArchetype.hpp"

struct EntityGroup {
	EntityArchetype accept;
	EntityArchetype reject;

	EntityGroup(EntityArchetype accept, EntityArchetype reject) {
		this->accept = accept;
		this->reject = reject;
	}
};

template<typename... Types>
EntityArchetype EntityGroupAccept() {
	return EntityArchetype(sizeof...(Types), { typeid(Types).hash_code()... }, { sizeof(Types)... }, { typeid(Types).name()... });
}

template<typename... Types>
EntityArchetype EntityGroupReject() {
	return EntityArchetype(sizeof...(Types), { typeid(Types).hash_code()... }, { sizeof(Types)... }, { typeid(Types).name()... });
}

#endif