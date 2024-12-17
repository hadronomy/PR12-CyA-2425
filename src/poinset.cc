/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingenierıa Informática
 * Asignatura: Computabilidad y Algoritmia
 * Curso: 4º
 * Práctica 11: Algoritmos Voraces (Greedy)
 * Grado en Ingeniería Informática
 * Computabilidad y Algoritmia
 * Autor: Pablo Hernández Jiménez
 * Correo: alu0101495934@ull.edu.es
 * Fecha: 19/09/2024
 * Archivo pointset.cc: Implementación de la clase PointSet
 * Referencias:
 */

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>

#include "cya/point_types.h"
#include "cya/pointset.h"

namespace cya {

void PointSet::EMST() {
  ArcVector arcs;
  ComputeArcVector(arcs);
  Forest forest;

  for (const Point& point : *this) {
    SubTree subTree;
    subTree.AddPoint(point);
    forest.emplace_back(std::move(subTree));
  }

  for (const WeightedArc& arc : arcs) {
    int i = -1, j = -1;
    FindIncidentSubtrees(forest, arc.second, i, j);
    if (i != j && i != -1 && j != -1) {
      MergeSubtrees(forest, arc.second, i, j);
    }
  }
  emst_ = forest[0].GetArcs();
}

void PointSet::EMSTImproved(int start_point) {
  emst_.clear();
  if (size() <= 1) {
    return;
  }

  // Find the closest point for each point greedily
  std::vector<bool> connected(size(), false);
  connected[start_point] = true;

  while (emst_.size() < size() - 1) {
    double min_distance = std::numeric_limits<double>::max();
    Arc closest_arc;
    int closest_index = -1;

    // Find the closest unconnected point to any connected point
    for (size_t i = 0; i < size(); ++i) {
      if (connected[i])
        continue;

      for (size_t j = 0; j < size(); ++j) {
        if (!connected[j])
          continue;

        double dist = EuclideanDistance(std::make_pair((*this)[i], (*this)[j]));
        if (dist < min_distance) {
          min_distance = dist;
          closest_arc = {(*this)[j], (*this)[i]};
          closest_index = i;
        }
      }
    }

    if (closest_index != -1) {
      emst_.push_back(closest_arc);
      connected[closest_index] = true;
      continue;
    }

    break;
  }
}

PointSet PointSet::EMSTMultistart() {
  PointSet& original = *this;
  PointSet& working_copy = original;
  PointSet& best_tree = original;
  for (size_t i = 0; i < size(); ++i) {
    working_copy.EMSTImproved(i);
    if (working_copy.ComputeCost() < best_tree.ComputeCost()) {
      best_tree = working_copy;
    }
    working_copy = original;
  }
  return best_tree;
}

void PointSet::ComputeArcVector(ArcVector& arcs) const {
  arcs.clear();
  const int n = size();
  for (int i = 0; i < n - 1; ++i) {
    const Point& p_i = (*this)[i];
    for (int j = i + 1; j < n; ++j) {
      const Point& p_j = (*this)[j];
      const double dist = EuclideanDistance(std::make_pair(p_i, p_j));
      arcs.emplace_back(dist, std::make_pair(p_i, p_j));
    }
  }
  std::sort(arcs.begin(), arcs.end());
}

void PointSet::FindIncidentSubtrees(const Forest& forest, const Arc& arc, int& i, int& j) const {
  i = j = 0;
  for (const SubTree& subtree : forest) {
    if (subtree.Contains(arc.first)) {
      i = &subtree - &forest[0];
    }
    if (subtree.Contains(arc.second)) {
      j = &subtree - &forest[0];
    }
  }
}

double PointSet::ComputeCost() const {
  double cost = 0.0;
  for (const Arc& arc : emst_) {
    cost += EuclideanDistance(arc);
  }
  return cost;
}

double PointSet::EuclideanDistance(const Arc& arc) const {
  const double dx = arc.first.x - arc.second.x;
  const double dy = arc.first.y - arc.second.y;
  return std::sqrt(dx * dx + dy * dy);
}

void PointSet::MergeSubtrees(Forest& forest, const Arc& arc, int i, int j) {
  forest[i].Merge(forest[j], std::make_pair(EuclideanDistance(arc), arc));
  forest.erase(forest.begin() + j);
}

void PointSet::QuickHull() {
  hull_.clear();

  Point min_x_point;
  Point max_x_point;

  XBounds(min_x_point, max_x_point);

  QuickHull(Line(min_x_point, max_x_point), Side::LEFT);
  QuickHull(Line(min_x_point, max_x_point), Side::RIGHT);

  // Remove duplicates
  std::sort(hull_.begin(), hull_.end());
  hull_.erase(std::unique(hull_.begin(), hull_.end()), hull_.end());
}

void PointSet::QuickHull(const Line& line, int side) {
  Point farthest;

  if (FarthestPoint(line, side, farthest)) {
    QuickHull(Line(line.first, farthest), -FindSide(Line(line.first, farthest), line.second));
    QuickHull(Line(farthest, line.second), -FindSide(Line(farthest, line.second), line.first));
  } else {
    hull_.push_back(line.first);
    hull_.push_back(line.second);
  }
}

bool PointSet::FarthestPoint(const Line& line, int side, Point& farthest) const {
  farthest = PointVector::at(0);
  double max_dist = 0;
  bool found = false;

  for (const Point& point : *this) {
    const double dist = Distance(line, point);

    if (FindSide(line, point) == side && dist > max_dist) {
      farthest = point;
      max_dist = dist;
      found = true;
    }
  }

  return found;
}

int PointSet::FindSide(const Line& line, const Point& p) const {
  double val = (p.y - line.first.y) * (line.second.x - line.first.x) -
               (p.x - line.first.x) * (line.second.y - line.first.y);
  if (val > 0)
    return 1;
  if (val < 0)
    return -1;
  return 0;
}

void PointSet::XBounds(Point& min_x, Point& max_x) const {
  min_x =
      *std::min_element(begin(), end(), [](const Point& a, const Point& b) { return a.x < b.x; });
  max_x =
      *std::max_element(begin(), end(), [](const Point& a, const Point& b) { return a.x < b.x; });
}

double PointSet::PointToLine(const Line& line, const Point& point) const {
  return std::abs((line.second.y - line.first.y) * point.x -
                  (line.second.x - line.first.x) * point.y + line.second.x * line.first.y -
                  line.second.y * line.first.x) /
         std::sqrt(std::pow(line.second.y - line.first.y, 2) +
                   std::pow(line.second.x - line.first.x, 2));
}

double PointSet::Distance(const Line& line, const Point& point) const {
  return fabs(PointToLine(line, point));
}

struct PointPtrComparator {
  bool operator()(const Point* a, const Point* b) const {
    if (a->x != b->x)
      return a->x < b->x;
    return a->y < b->y;
  }
};

void PointSet::WriteDot(const std::string& filename) const {
  // Open file with error checking
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file: " + filename + " for writing.");
  }

  // Start dot graph with minimalist styling
  file << "graph G {\n";
  file << "  layout=neato;\n";
  file << "  overlap=false;\n";
  file << "  bgcolor=white;\n";

  // Node styling at the graph level
  file << "  node [style=filled, color=black, fillcolor=black, width=0.05, height=0.05, "
          "shape=point];\n";

  // Map points to unique numerical indices
  std::map<Point*, int, PointPtrComparator> point_indices;
  int counter = 0;

  for (const Point& point : *this) {
    if (point_indices.find(const_cast<Point*>(&point)) == point_indices.end()) {
      point_indices[const_cast<Point*>(&point)] = counter++;
    }
  }

  for (const auto& pair : point_indices) {
    const Point& point = *pair.first;
    int index = pair.second;

    file << "  " << index << " [\n";
    file << "    pos=\"" << std::fixed << std::setprecision(2) << point.x << "," << point.y
         << "!\"\n";
    file << "  ];\n";
  }

  std::vector<Point> sorted_hull = hull_;
  std::sort(sorted_hull.begin(), sorted_hull.end(), [this](const Point& a, const Point& b) {
    return std::atan2(a.y, a.x) < std::atan2(b.y, b.x);
  });

  for (size_t i = 0; i < sorted_hull.size(); ++i) {
    const Point& point = sorted_hull[i];
    const Point& next_point = sorted_hull[(i + 1) % sorted_hull.size()];

    int index = point_indices.at(const_cast<Point*>(&point));
    int next_index = point_indices.at(const_cast<Point*>(&next_point));

    file << "  " << index << " -- " << next_index << " [\n";
    file << "    color=\"black\"\n";
    file << "    penwidth=1\n";
    file << "  ];\n";
  }

  file << "}\n";
  file.close();

  if (!file.good()) {
    throw std::runtime_error("Error occurred while writing to file: " + filename);
  }
}

void PointSet::Write(const std::string& filename) const {
  std::ofstream file(filename);

  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for writing.");
  }

  for (const Point& point : hull_) {
    file << "(" << point.x << ", " << point.y << ")";
    file << std::endl;
  }
}

}  // namespace cya