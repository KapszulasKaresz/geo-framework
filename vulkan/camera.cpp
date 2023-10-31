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
    pos[0] += amount * forward.x() * scale;
    pos[2] += amount * forward.z() * scale;
}

void Camera::strafe(float amount)
{
    pos[0] += amount * right.x() * scale;
    pos[2] += amount * right.z() * scale;
}

void Camera::fly(float amount)
{
    pos[1] += amount * up.y() * scale;
}

void Camera::move(QVector3D amount)
{
    pos += amount * scale;
}

void Camera::updateCameraBasedOnBoundingBox(const QVector3D& bottomrleft, const QVector3D& topright)
{
    bounding_bl = bottomrleft;
    bounding_tr = topright;
    scale = (bottomrleft - topright).length();

    pos = (bottomrleft + topright) / 2;
    pos.setZ(topright.z() + 3* topright.z());

    m_yaw = 0;
    m_pitch = 0;
    pitch(0);
    yaw(0);
}

void Camera::standardX()
{
    pos = (bounding_bl + bounding_tr) / 2;
    pos.setZ(bounding_tr.z() + 3 * bounding_tr.z());

    m_yaw = 0;
    m_pitch = 0;
    pitch(0);
    yaw(0);
}

void Camera::standardY()
{
    pos = (bounding_bl + bounding_tr) / 2;
    pos.setY(bounding_tr.y() + 3 * bounding_tr.y());

    m_yaw = 0;
    m_pitch = 0;
    pitch(90);
    yaw(0);
}

void Camera::standardZ()
{
    pos = (bounding_bl + bounding_tr) / 2;
    pos.setX(bounding_tr.x() + 3 * bounding_tr.x());

    m_yaw = 0;
    m_pitch = 0;
    pitch(0);
    yaw(270);
}

QVector3D Camera::getRayDir(int X, int Y, QSize ScreenSize)
{
    QVector3D r = right.normalized() * (ScreenSize.width() / ScreenSize.height());
    QVector3D u = QVector3D::crossProduct(forward, r).normalized();
    return forward + r * (2.0f * (X + 0.5f) / ScreenSize.width() - 1) + u * (2.0f * (Y + 0.5f) / ScreenSize.height() - 1);
}
