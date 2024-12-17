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

#pragma once

#include <ostream>
#include <set>
#include <utility>
#include <vector>

namespace cya {

const int MAX_SIZE = 3;
const int MAX_PRECISION = 0;

struct Point {
  double x;
  double y;

  bool operator==(const Point& other) const { return x == other.x && y == other.y; }
  bool operator!=(const Point& other) const { return !(*this == other); }
  bool operator<(const Point& other) const { return x < other.x || (x == other.x && y < other.y); }
  bool operator>(const Point& other) const { return x > other.x || (x == other.x && y > other.y); }
  bool operator<=(const Point& other) const { return *this < other || *this == other; }
  bool operator>=(const Point& other) const { return *this > other || *this == other; }

  Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
  Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
  Point operator*(const double scalar) const { return {x * scalar, y * scalar}; }
  Point operator/(const double scalar) const { return {x / scalar, y / scalar}; }
  double operator*(const Point& other) const { return x * other.x + y * other.y; }

  friend std::ostream& operator<<(std::ostream& os, const Point& ps);
  friend std::istream& operator>>(std::istream& is, Point& ps);
};

using Line = std::pair<Point, Point>;
using PointVector = std::vector<Point>;
using Arc = Line;
using WeightedArc = std::pair<double, Arc>;
using ArcVector = std::vector<WeightedArc>;
using PointCollection = std::set<Point>;
using Tree = std::vector<Arc>;

enum Side { LEFT = -1, CENTER, RIGHT };

std::ostream& operator<<(std::ostream& os, const PointVector& ps);
std::istream& operator>>(std::istream& is, PointVector& ps);

};  // namespace cya
