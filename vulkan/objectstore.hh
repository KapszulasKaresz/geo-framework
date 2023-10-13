#pragma once
#include "object.hh"


class ObjectStore {
	std::vector<Object*> objects;

public:
	std::vector<Object*>& getObjects() { return objects; }
	void addObject(Object* object) { objects.push_back(object); }
	void clear() { objects.clear(); }
	int getVerticieCount();
	float* getVertexData();

	~ObjectStore() { clear(); }
};