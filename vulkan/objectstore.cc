#include "objectstore.hh"

int ObjectStore::getVerticieCount()
{
	int ret = 0;
	for (auto object : objects) {
		ret += object->getVerticieCount();
	}

	return ret;
}

float* ObjectStore::getVertexData()
{

	float* ret = new float[getVerticieCount() * 8];
	int i = 0;

	for (int k = 0; k < objects.size(); k++) {
		float* copy = objects[k]->getVertexData();
		for (int j = 0; j < (objects[k]->getVerticieCount() * 8); j++) {
			ret[i++] = copy[j];
		}
		delete copy;
	}

	return ret;
}
