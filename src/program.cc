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
 * Archivo program.cc: Program class implementation.
 * Referencias:
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include "cya/cli.h"
#include "cya/parser.h"
#include "cya/point_types.h"
#include "cya/pointset.h"
#include "cya/program.h"
#include "mitata.h"

namespace cya {

static const std::string kDescription = R"(
  Uses the Kruskal algorithm to compute 
  the Euclidean Minimum Spanning Tree (EMST) of a set of points.
)";

static const std::string kExampleFile = R"(10
68 -21
57 60
82 -60
-33	54
-44	11
-5 26
-27	3
90 83
27 43
-72	21
)";

std::string ReadFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

/**
 * @brief Runs the Program.
 *
 * This method parses the command line arguments, reads the input file, and
 * executes the specified operation on the input strings. The results are
 * written to the output file.
 */
void Program::Run() {
  cli::ArgumentParser cli("cya", kDescription);
  cli.AddPositionalArgument("input", "Input file").End();
  cli.AddPositionalArgument("output", "Output file").End();
  cli.AddArgument("dot", "d", "Set output format to graphviz `.dot`")
      .SetFlag()
      .SetDefaultValue(false)
      .End();
  cli.AddArgument("order", "o", "Prints the order of a point").SetMultiple(2).End();
  cli.AddArgument("bench", "b", "Run benchmarks").SetFlag().SetDefaultValue(false).End();
  cli.AddArgument("improved", "i", "Use improved algorithm").SetFlag().SetDefaultValue(false).End();

  try {
    cli.Parse(arguments_);
    const std::string& input_filename = cli.GetValue<std::string>("input");
    const std::string& output_filename = cli.GetValue<std::string>("output");
    const std::string input = ReadFile(input_filename);

    if (cli.GetValue<bool>("bench")) {
      RunBenchmarks();
      return;
    }
    ProcessInput(input, output_filename, cli);
  } catch (const cli::CliParserError& e) {
    return;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

void Program::RunBenchmarks() {
  auto points_result = ParsePointsFromString(kExampleFile);
  if (!points_result) {
    throw std::runtime_error(std::string("Error parsing points: ") + points_result.error().what());
  }
  const PointVector& points = points_result.value();

  mitata::runner runner;

  runner.summary([&]() {
    runner.bench("Normal", [&]() { Process(points); });
    runner.bench("Improved", [&]() { ProcessImproved(points); });
  });

  auto stats = runner.run();
}

void Program::ProcessInput(const std::string& input, const std::string& output_filename,
                           const cli::ArgumentParser& cli) {
  auto points_result = ParsePointsFromString(input);
  if (!points_result) {
    throw std::runtime_error(std::string("Error parsing points: ") + points_result.error().what());
  }
  const PointVector& points = points_result.value();
  std::optional<PointSet> processed_points;
  if (cli.GetValue<bool>("improved")) {
    processed_points = ProcessImproved(points);
  } else {
    processed_points = Process(points);
  }

  if (cli.WasArgumentPassed("order")) {
    const std::vector<std::string> point_vector = cli.GetValue<std::vector<std::string>>("order");
    if (point_vector.size() != 2) {
      throw std::runtime_error("Expected 2 values for --order argument");
    }
    std::vector<double> point_vector_double;
    for (const auto& point : point_vector) {
      try {
        point_vector_double.push_back(std::stod(point));
      } catch (const std::exception& e) {
        throw std::runtime_error("Invalid point value: " + point);
      }
    }

    Point point = {static_cast<double>(point_vector_double[0]),
                   static_cast<double>(point_vector_double[1])};
    std::cout << "Order of point " << point.x << ", " << point.y << ": "
              << processed_points.value().GetPointOrder(point) << std::endl;
  }

  if (cli.GetValue<bool>("dot")) {
    processed_points.value().WriteDot(output_filename);
    return;
  }
  processed_points.value().Write(output_filename);
}

PointSet Program::Process(const PointVector& points) {
  PointSet point_set(points);
  point_set.QuickHull();
  return point_set;
}

PointSet Program::ProcessImproved(const PointVector& points) {
  PointSet point_set(points);
  point_set.QuickHull();
  return point_set;
}

}  // namespace cya
