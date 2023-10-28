#pragma once
#include "object.hh"
#include "mesh.hh"
#include "qvector3d.h"


class ObjectStore {
	std::vector<Object*> objects;
	std::vector<Mesh*> controlPoints;

	Mesh* ControlPoint = new Mesh("D:/Temalabor/models/cube.obj");

	float meanMin;
	float meanMax;
	float cutoffRatio = 0.1f;

	int selectedObjectID = -1;
	int selectedVertexID = -1;

	QVector3D movementAxis = QVector3D(1, 0, 0);

public:
	std::vector<Object*>& getObjects() { return objects; }
	void addObject(Object* object);
	void clear() { objects.clear(); controlPoints.clear(); }
	int getVerticieCount();
	int getVerticieCountCP();
	float* getVertexDataCP();
	float* getVertexData();
	void updateMeanMinMax();
	void updateMesh();
	void updateSelected(QVector3D from, QVector3D dir);
	void moveSelected(QVector3D from, QVector3D dir);
	double getCutoffRatio() const { return cutoffRatio; }
	void setCutoffRatio(double ratio) { cutoffRatio = ratio; updateMeanMinMax(); }
	double getMeanMin() const { return meanMin; }
	void setMeanMin(double min) { meanMin = min; updateMesh(); }
	double getMeanMax() const { return meanMax; }
	void setMeanMax(double max) { meanMax = max; updateMesh(); }
	void setMovementAxis(QVector3D axis) { movementAxis = axis; }
	~ObjectStore() { clear(); }

private:
	void updatePoints();
	QVector3D getNegatedAxis();
};