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
 * Archivo pointset.h: Declaración de la clase PointSet
 * Referencias:
 */

#pragma once

#include "cya/point_types.h"
#include "cya/subtree.h"

namespace cya {

using Forest = std::vector<SubTree>;

class PointSet : public PointVector {
 public:
  PointSet(const PointVector& points) : PointVector(points) {}

  void EMST();
  void EMSTImproved(int start_point = 0);
  PointSet EMSTMultistart();
  void QuickHull();

  void WriteDot(const std::string& filename) const;
  void Write(const std::string& filename) const;

  inline const Tree& GetTree() const { return emst_; }
  inline const PointVector& GetPoints() const { return *this; }
  inline const double GetCost() const { return ComputeCost(); }
  inline const PointVector& GetHull() const { return hull_; }
  inline const int GetPointOrder(const Point& point) const {
    int order = 0;
    for (const auto& arc : emst_) {
      if (arc.first == point || arc.second == point) {
        ++order;
      }
    }
    return order;
  }

 private:
  void QuickHull(const Line& line, int side);
  void ComputeArcVector(ArcVector& arcs) const;
  void FindIncidentSubtrees(const Forest& forest, const Arc& arc, int& i, int& j) const;
  void MergeSubtrees(Forest& forest, const Arc& arc, int i, int j);
  int FindSide(const Line& line, const Point& p) const;
  void XBounds(Point& min_x, Point& max_x) const;
  double PointToLine(const Line& line, const Point& point) const;
  bool FarthestPoint(const Line& line, int side, Point& farthest) const;
  double Distance(const Line& line, const Point& point) const;

  double ComputeCost() const;

  double EuclideanDistance(const Arc& arc) const;

 private:
  Tree emst_;
  PointVector hull_;
};

}  // namespace cya