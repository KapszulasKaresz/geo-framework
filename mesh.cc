#include <OpenMesh/Core/IO/MeshIO.hh>

#include "mesh.hh"

Mesh::Mesh(std::string filename) : Object(filename) {
  reload();
}

Mesh::~Mesh() {
}

void Mesh::drawWithNames(const Visualization &vis) const {
  if (!vis.show_wireframe)
    return;
  for (auto v : mesh.vertices()) {
    glPushName(v.idx());
    glRasterPos3dv(mesh.point(v).data());
    glPopName();
  }
}

Vector Mesh::postSelection(int selected) {
  return mesh.point(BaseMesh::VertexHandle(selected));
}

void Mesh::movement(int selected, const Vector &pos) {
  mesh.set_point(BaseMesh::VertexHandle(selected), pos);
}



void Mesh::getClosest(int& id, float& distance, QVector3D from, QVector3D dir)
{
    float minDist = std::numeric_limits<float>::max();
    int idx = -1;

    for (auto v : mesh.vertices()) { 
        QVector3D point = QVector3D(mesh.point(v).data()[0], mesh.point(v).data()[1], mesh.point(v).data()[2]); 
        float dist = point.distanceToLine(from, dir);  
        if (dist < minDist) { 
            minDist = dist; 
            idx = v.idx();
        }
    }

    distance = minDist;
    id = idx;
}

bool Mesh::reload() {
  if (!OpenMesh::IO::read_mesh(mesh, filename))
    return false;
  updateBaseMesh();
  return true;
}
