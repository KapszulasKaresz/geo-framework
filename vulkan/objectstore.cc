#include "objectstore.hh"
#include "qvector.h"
#include "qdebug.h"
#include "qline.h"
#include "qmatrix4x4.h"
#include "qpoint.h"

void ObjectStore::addObject(Object* object)
{

	objects.push_back(object);

	double large = std::numeric_limits<double>::max();
	Vector box_min(large, large, large), box_max(-large, -large, -large);
	const auto& mesh = object->baseMesh();
	for (auto v : object->baseMesh().vertices()) {
		box_min.minimize(mesh.point(v));
		box_max.maximize(mesh.point(v));
	}

	Vector move = -(box_max + box_min) / 2;

	for (int i = 0; i < object->pointstoUpdate(); i++) {
		Vector p = object->postSelection(i);
		object->movement(i, p + move);
	}

	updateMeanMinMax();
}

void ObjectStore::updateControlPoints(Object* object, float boundingBoxSize) {
	this->boundingBoxSize = boundingBoxSize;
	for (int i = 0; i < object->controlPoints(); i++) {
		controlPoints.push_back(new Mesh("D:/Temalabor/models/cube.obj"));
		Vector p = Vector(object->postSelection(i).data()[0], object->postSelection(i).data()[1], object->postSelection(i).data()[2]);
		for (int j = 0; j < controlPoints[controlPoints.size() - 1]->baseMesh().n_vertices(); j++) {
			Vector p2 = Vector(controlPoints[controlPoints.size() - 1]->postSelection(j).data()[0], controlPoints[controlPoints.size() - 1]->postSelection(j).data()[1], controlPoints[controlPoints.size() - 1]->postSelection(j).data()[2]);
			p2 *= 1 / (boundingBoxSize * 3);
			controlPoints[controlPoints.size() - 1]->movement(j, p + p2 - Vector(1 / (boundingBoxSize * 6), 1 / (boundingBoxSize * 6), 1 / (boundingBoxSize * 6)));
		}
	}
}

int ObjectStore::getVerticieCount()
{
	int ret = 0;
	for (auto object : objects) {
		ret += object->getVerticieCount();
	}

	return ret;
}

int ObjectStore::getVerticieCountCP()
{
	int ret = 0;

	for (auto point : controlPoints) {
		ret += point->getVerticieCount();
	}
	
	if (ret == 0) {
		return 1;
	}

	return ret;
}


int ObjectStore::getVerticieCountCPLine() {
	int ret = 0; 

	for (auto object : objects) {
		if (object->controlPoints() > 0) {
			size_t* degree = object->getDegree();
			ret += degree[0] * degree[1] * 4;
		}
	}
	
	if (ret == 0)
		return 1;

	return ret;
}

float* ObjectStore::getVertexDataCP()
{

	float* ret = new float[getVerticieCountCP() * 3];
	int i = 0;

	for (int k = 0; k < controlPoints.size(); k++) {
		float* copy = controlPoints[k]->getVertexData(meanMin, meanMax);
		for (int j = 0; j < (controlPoints[k]->getVerticieCount() * 9); j+=9) {
			ret[i++] = copy[j];
			ret[i++] = copy[j + 1];
			ret[i++] = copy[j + 2];
		}
		delete copy;
	}


	return ret;
}

float* ObjectStore::getVertexDataCPLine() {
	float* ret = new float[getVerticieCountCPLine() * 3];
	int l = 0;

	for (auto object : objects) {
		if (object->controlPoints() > 0) {
			size_t* degree = object->getDegree();
			size_t m = degree[1] + 1;
			for (size_t k = 0; k < 2; ++k)
				for (size_t i = 0; i <= degree[k]; ++i) {
					for (size_t j = 0; j <= degree[1 - k]; ++j) {
						size_t const index = k ? j * m + i : i * m + j;
						const auto& p = object->postSelection(index);
						ret[l++] = p.data()[0];
						ret[l++] = p.data()[1];
						ret[l++] = p.data()[2];
					}
				}
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
		objects[i]->getClosest(id, distance, from, dir, model);
		if (distance < minDist) {
			minDist = distance;
			selectedObjectID = i;
			selectedVertexID = id;
		}
	}
}

QVector3D ObjectStore::getNegatedAxis() {
	return QVector3D(movementAxis.x() > 0 ? 0:1, movementAxis.y() > 0 ? 0:1, movementAxis.z() > 0? 0:1);
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
				p2 *= 1 / (boundingBoxSize * 3);
				controlPoints[controlPoints.size() - 1]->movement(j, p + p2 - Vector(1 / (boundingBoxSize * 6), 1 / (boundingBoxSize * 6), 1 / (boundingBoxSize * 6)));
			}
		}
	}
}
