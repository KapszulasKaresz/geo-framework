#include "objectstore.hh"
#include "qvector.h"
#include "qdebug.h"
#include "qline.h"
#include "qpoint.h"

void ObjectStore::addObject(Object* object)
{
	objects.push_back(object);
	for (int i = 0; i < object->controlPoints(); i++) {
		controlPoints.push_back(new Mesh("D:/Temalabor/models/cube.obj"));
		Vector p = Vector(object->postSelection(i).data()[0], object->postSelection(i).data()[1], object->postSelection(i).data()[2]);
		for (int j = 0; j < controlPoints[controlPoints.size() - 1]->baseMesh().n_vertices(); j++) {
			Vector p2 = Vector(controlPoints[controlPoints.size() - 1]->postSelection(j).data()[0], controlPoints[controlPoints.size() - 1]->postSelection(j).data()[1], controlPoints[controlPoints.size() - 1]->postSelection(j).data()[2]);
			controlPoints[controlPoints.size() - 1]->movement(j, p + p2);
		}
	}
	updateMeanMinMax();
}

int ObjectStore::getVerticieCount()
{
	int ret = 0;
	for (auto object : objects) {
		ret += object->getVerticieCount();
	}

	if (showControlPoints) {
		for (auto point : controlPoints) {
			ret += point->getVerticieCount();
		}
	}

	return ret;
}

float* ObjectStore::getVertexData()
{

	float* ret = new float[getVerticieCount() * 9];
	int i = 0;

	for (int k = 0; k < objects.size(); k++) {
		float* copy = objects[k]->getVertexData(meanMin, meanMax);
		for (int j = 0; j < (objects[k]->getVerticieCount() * 9); j++) {
			ret[i++] = copy[j];
		}
		delete copy;
	}

	if (showControlPoints) {
		for (int k = 0; k < controlPoints.size(); k++) {
			float* copy = controlPoints[k]->getVertexData(meanMin, meanMax);
			for (int j = 0; j < (controlPoints[k]->getVerticieCount() * 9); j++) {
				ret[i++] = copy[j];
			}
			delete copy;
		}
	}

	return ret;
}

void ObjectStore::updateMeanMinMax()
{
	std::vector<double> mean;
	for (auto o : objects) {
		const auto& mesh = o->baseMesh();
		for (auto v : mesh.vertices())
			mean.push_back(mesh.data(v).mean);
	}

	size_t n = mean.size();
	if (n < 3)
		return;

	std::sort(mean.begin(), mean.end());
	size_t k = (double)n * cutoffRatio;
	meanMin = std::min(mean[k ? k - 1 : 0], 0.0);
	meanMax = std::max(mean[k ? n - k : n - 1], 0.0);
	updateMesh();
}

void ObjectStore::updateMesh()
{
	for (auto o : objects) {
		o->updateBaseMesh();
	}
}

void ObjectStore::updateSelected(QVector3D from, QVector3D dir)
{
	float minDist = std::numeric_limits<float>::max();
	for (int i = 0; i < objects.size(); i++) {
		float distance;
		int id;
		objects[i]->getClosest(id, distance, from, dir);
		if (distance < minDist) {
			minDist = distance;
			selectedObjectID = i;
			selectedVertexID = id;
		}
	}
}

void ObjectStore::moveSelected(QVector3D from, QVector3D dir)
{
	Vector p = objects[selectedObjectID]->postSelection(selectedVertexID);
	QVector3D point = QVector3D(p.data()[0], p.data()[1], p.data()[2]);
	
	float d = QVector3D::dotProduct(point - from, dir);

	QVector3D pOnLine = from + dir.normalized() * d;

	QVector3D intersection = (pOnLine - point) * movementAxis + point;

	objects[selectedObjectID]->movement(selectedVertexID, Vector(intersection.x(), intersection.y(), intersection.z()));
	objects[selectedObjectID]->updateBaseMesh();

	updatePoints();

	updateMeanMinMax();
	
}

void ObjectStore::updatePoints()
{
	controlPoints.clear();
	for (auto object : objects) {
		for (int i = 0; i < object->controlPoints(); i++) {
			controlPoints.push_back(new Mesh("D:/Temalabor/models/cube.obj"));
			Vector p = Vector(object->postSelection(i).data()[0], object->postSelection(i).data()[1], object->postSelection(i).data()[2]);
			for (int j = 0; j < controlPoints[controlPoints.size() - 1]->baseMesh().n_vertices(); j++) {
				Vector p2 = Vector(controlPoints[controlPoints.size() - 1]->postSelection(j).data()[0], controlPoints[controlPoints.size() - 1]->postSelection(j).data()[1], controlPoints[controlPoints.size() - 1]->postSelection(j).data()[2]);
				controlPoints[controlPoints.size() - 1]->movement(j, p + p2);
			}
		}
	}
}
