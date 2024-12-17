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
 * Archivo program.h: Program class header
 * Referencias:
 */

#pragma once

#include <string>
#include <vector>

#include "cya/cli.h"
#include "cya/point_types.h"

namespace cya {

class PointSet;

/**
 * @brief Main program class
 *
 */
class Program {
 public:
  Program(const int argc, const char* argv[]) : arguments_(argv + 1, argv + argc) {}

  void Run();

 private:
  void RunBenchmarks();
  void ProcessInput(const std::string& input, const std::string& output_filename,
                    const cli::ArgumentParser& parser);
  PointSet Process(const PointVector& points);
  PointSet ProcessImproved(const PointVector& points);
  PointSet ProcessMultistart(const PointVector& points);

  std::vector<std::string> arguments_;
};

}  // namespace cya
