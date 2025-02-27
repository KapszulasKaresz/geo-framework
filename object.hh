#pragma once

#include "base-mesh.hh"
#include "visualization.hh"
#include "qvector3d.h"

class Object {
public:
  explicit Object(std::string filename);
  virtual ~Object();
  const BaseMesh &baseMesh() const;
  float* getVertexData(double meanMin, double meanMax);
  int getVerticieCount() const;
  virtual void draw(const Visualization &vis) const;
  virtual void drawWithNames(const Visualization &vis) const = 0;
  virtual Vector postSelection(int selected) = 0;
  virtual void movement(int selected, const Vector &pos) = 0;
  virtual void updateBaseMesh();
  virtual bool reload() = 0;
  virtual int controlPoints();
  virtual int pointstoUpdate() = 0;
  virtual size_t* getDegree() { return NULL; }
  virtual void getClosest(int& id, float& distance, QVector3D from, QVector3D dir, QMatrix4x4 model) = 0;
  bool valid() const;
protected:
  void updateBaseMesh(bool own_normal, bool own_mean);
  virtual Vector normal(BaseMesh::VertexHandle vh) const;
  virtual double meanCurvature(BaseMesh::VertexHandle vh) const;
  Vector colorMap(double min, double max, double d);

  BaseMesh mesh;
  std::string filename;
};
