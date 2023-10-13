#include "camera.hh"

QMatrix4x4 Camera::viewMatrix() const
{
    QMatrix4x4 m = pitchMatrix * yawMatrix;
    m.translate(-pos);
    return m;
}

static inline void clamp360(float* v)
{
    if (*v > 360.0f)
        *v -= 360.0f;
    if (*v < -360.0f)
        *v += 360.0f;
}

void Camera::yaw(float degrees)
{
    m_yaw += degrees;
    clamp360(&m_yaw);
    yawMatrix.setToIdentity();
    yawMatrix.rotate(m_yaw, 0, 1, 0);

    QMatrix4x4 rotMat = pitchMatrix * yawMatrix;
    forward = (QVector4D(0.0f, 0.0f, -1.0f, 0.0f) * rotMat).toVector3D().normalized();
    right = (QVector4D(1.0f, 0.0f, 0.0f, 0.0f) * rotMat).toVector3D().normalized();
}

void Camera::pitch(float degrees)
{
    m_pitch += degrees;
    clamp360(&m_pitch);
    pitchMatrix.setToIdentity();
    pitchMatrix.rotate(m_pitch, 1, 0, 0);

    QMatrix4x4 rotMat = pitchMatrix * yawMatrix;
    forward = (QVector4D(0.0f, 0.0f, -1.0f, 0.0f) * rotMat).toVector3D().normalized();
    up = (QVector4D(0.0f, 1.0f, 0.0f, 0.0f) * rotMat).toVector3D().normalized();
}

void Camera::walk(float amount)
{
    pos[0] += amount * forward.x();
    pos[2] += amount * forward.z();
}

void Camera::strafe(float amount)
{
    pos[0] += amount * right.x();
    pos[2] += amount * right.z();
}

void Camera::fly(float amount)
{
    pos[1] += amount * up.y();
}

void Camera::move(QVector3D amount)
{
    pos += amount;
}

void Camera::updateCameraBasedOnBoundingBox(const QVector3D& bottomrleft, const QVector3D& topright)
{
    bounding_bl = bottomrleft;
    bounding_tr = topright;

    pos = (bottomrleft + topright) / 2;
    pos.setZ(topright.z() + 2* topright.z());

    m_yaw = 0;
    m_pitch = 0;
    pitch(0);
    yaw(0);
}

void Camera::standardX()
{
    pos = (bounding_bl + bounding_tr) / 2;
    pos.setZ(bounding_tr.z() + 2 * bounding_tr.z());

    m_yaw = 0;
    m_pitch = 0;
    pitch(0);
    yaw(0);
}

void Camera::standardY()
{
    pos = (bounding_bl + bounding_tr) / 2;
    pos.setY(bounding_tr.y() + 2 * bounding_tr.y());

    m_yaw = 0;
    m_pitch = 0;
    pitch(90);
    yaw(0);
}

void Camera::standardZ()
{
    pos = (bounding_bl + bounding_tr) / 2;
    pos.setX(bounding_tr.x() + 2 * bounding_tr.x());

    m_yaw = 0;
    m_pitch = 0;
    pitch(0);
    yaw(270);
}