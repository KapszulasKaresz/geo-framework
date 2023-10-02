#pragma once
#include <qvector3d.h>
#include <qmatrix4x4.h>

class Camera {
	QVector3D forward, right, up, pos;
	float yaw, pitch;

public:
	Camera(const QVector3D& pos) {}

	QMatrix4x4 viewMatrix() const {}

};