#pragma once

#include "object.hh"

class Bezier : public Object {
public:
  Bezier(std::string filename);
  virtual ~Bezier();
  virtual void draw(const Visualization &vis) const override;
  virtual void drawWithNames(const Visualization &vis) const override;
  virtual void getClosest(int& id, float& distance, QVector3D from, QVector3D dir, QMatrix4x4 model) override;
  virtual Vector postSelection(int selected) override;
  virtual void movement(int selected, const Vector &pos) override;
  virtual void updateBaseMesh() override;
  virtual int controlPoints() override { return control_points.size(); }
  virtual int pointstoUpdate() override { return controlPoints(); }
  virtual bool reload() override;
  virtual size_t* getDegree() override { return degree; }
private:
  size_t degree[2];
  std::vector<Vector> control_points;
};
