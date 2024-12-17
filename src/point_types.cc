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
 * Archivo point_types.cc: Implementación de las clases Point y PointVector
 * Referencias:
 */

#include <iomanip>
#include <iostream>

#include "cya/point_types.h"

namespace cya {

std::ostream& operator<<(std::ostream& os, const PointVector& points) {
  os << points.size() << std::endl;
  for (const Point& point : points) {
    os << point << std::endl;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Point& point) {
  os << std::setw(MAX_SIZE) << std::fixed << std::setprecision(MAX_PRECISION) << point.x << "\t"
     << std::setw(MAX_SIZE) << std::fixed << std::setprecision(MAX_PRECISION) << point.y;

  return os;
}

std::istream& operator>>(std::istream& is, PointVector& points) {
  int number;
  is >> number;
  points.clear();

  for (int i = 0; i < number; ++i) {
    Point point;
    is >> point;
    points.push_back(point);
  }
  return is;
}

std::istream& operator>>(std::istream& is, Point& point) {
  is >> point.x >> point.y;
  return is;
}

}  // namespace cya
