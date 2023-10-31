#pragma once

#include "object.hh"

class Mesh : public Object {
public:
  Mesh(std::string filename);
  virtual ~Mesh();
  virtual void drawWithNames(const Visualization &vis) const override;
  virtual Vector postSelection(int selected) override;
  virtual void movement(int selected, const Vector &pos) override;
  virtual void getClosest(int& id, float& distance, QVector3D from, QVector3D dir, QMatrix4x4 model) override;
  virtual bool reload() override;
  virtual int pointstoUpdate() override;
};
