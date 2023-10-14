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
