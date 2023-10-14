#pragma once
#include "object.hh"


class ObjectStore {
	std::vector<Object*> objects;

	float meanMin;
	float meanMax;
	float cutoffRatio = 0.1f;


public:
	std::vector<Object*>& getObjects() { return objects; }
	void addObject(Object* object) { objects.push_back(object); updateMeanMinMax(); }
	void clear() { objects.clear(); }
	int getVerticieCount();
	float* getVertexData();
	void updateMeanMinMax();
	void updateMesh();

	double getCutoffRatio() const { return cutoffRatio; }
	void setCutoffRatio(double ratio) { cutoffRatio = ratio; updateMeanMinMax(); }
	double getMeanMin() const { return meanMin; }
	void setMeanMin(double min) { meanMin = min; updateMesh(); }
	double getMeanMax() const { return meanMax; }
	void setMeanMax(double max) { meanMax = max; updateMesh(); }


	~ObjectStore() { clear(); }
};