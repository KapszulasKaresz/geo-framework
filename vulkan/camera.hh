#pragma once
#include <qvector3d.h>
#include <qmatrix4x4.h>

class Camera {
	QVector3D forward, right, up, pos, bounding_bl, bounding_tr;
	float m_yaw;
	float m_pitch;
	QMatrix4x4 yawMatrix, pitchMatrix;

public:
	Camera(const QVector3D& pos) : forward(0.0f, 0.0f, -1.0f),
		right(1.0f, 0.0f, 0.0f),
		up(0.0f, 1.0f, 0.0f),
		pos(pos),
		m_yaw(0.0f),
		m_pitch(0.0f) {
		yaw(0);
		pitch(0);
	}

	void yaw(float degrees);
	void pitch(float degrees);
	void walk(float amount);
	void strafe(float amount);
	void fly(float amount);
	void move(QVector3D amount);
	void setPos(QVector3D _pos) { pos = _pos; }
	void updateCameraBasedOnBoundingBox(const QVector3D& bottomleft, const QVector3D& topright);
	void standardX();
	void standardY();
	void standardZ();
	QVector3D getForward() const { return forward; }

	QMatrix4x4 viewMatrix() const;

};