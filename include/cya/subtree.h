
#pragma once

#include "cya/point_types.h"
namespace cya {

class SubTree {
 public:
  SubTree() = default;

  void AddArc(const Arc& arc, const double cost) {
    arcs_.emplace_back(arc);
    points_.emplace(arc.first);
    points_.emplace(arc.second);
  }

  void AddPoint(const Point& point) { points_.emplace(point); }

  bool Contains(const Point& point) const { return points_.count(point); }

  void Merge(const SubTree& other, const WeightedArc& arc) {
    arcs_.insert(arcs_.end(), other.GetArcs().begin(), other.GetArcs().end());
    arcs_.emplace_back(arc.second);
    points_.insert(other.GetPoints().begin(), other.GetPoints().end());
    cost_ += arc.first + other.GetCost();
  }

  inline const Tree& GetArcs() const { return arcs_; }
  inline const PointCollection& GetPoints() const { return points_; }
  inline double GetCost() const { return cost_; }

 private:
  Tree arcs_;
  PointCollection points_;
  double cost_;
};

using SubTreeVector = std::vector<SubTree>;

}  // namespace cya