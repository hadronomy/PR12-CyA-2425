/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingenierıa Informática
 * Asignatura: Computabilidad y Algoritmia
 * Curso: 4º
 * Práctica 12: Divide y Vencerás
 * Grado en Ingeniería Informática
 * Computabilidad y Algoritmia
 * Autor: Pablo Hernández Jiménez
 * Correo: alu0101495934@ull.edu.es
 * Fecha: 19/09/2024
 * Archivo parser.h: Implementación de un parser de puntos
 * Referencias:
 */

#pragma once

#include <algorithm>
#include <charconv>
#include <concepts>
#include <expected>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "point_types.h"

namespace cya {

// Advanced ParseError with rich error reporting
class ParseError : public std::runtime_error {
 public:
  ParseError(const std::string& message, int line, int column, const std::string& context,
             const std::string& token)
      : std::runtime_error(message),
        line_(line),
        column_(column),
        message_(message),
        context_(context),
        token_(token) {}

  const char* what() const noexcept override {
    static std::string full_message;
    const std::string red = "\033[1;31m";
    const std::string yellow = "\033[1;33m";
    const std::string reset = "\033[0m";
    const auto& line = std::to_string(line_);
    int pointer_size = token_.empty() ? 1 : token_.size();

    full_message.clear();
    full_message += red + "error: " + message_ + reset + "\n";
    full_message +=
        yellow + " --> line " + line + ", column " + std::to_string(column_) + reset + "\n";
    full_message += std::string(line.size(), ' ') + " |\n";
    full_message += line + " | " + context_ + "\n";
    int space_count = std::max(0, column_ - static_cast<int>(token_.size()) - 1);
    full_message += std::string(line.size(), ' ') + " | " + std::string(space_count, ' ') + red +
                    std::string(pointer_size, '^') + "~~~Here" + reset + "\n";
    return full_message.c_str();
  }

 private:
  int line_;
  int column_;
  std::string message_;
  std::string context_;
  std::string token_;
};

// Concept to constrain point-like types
template <typename T>
concept PointType = requires(T a) {
  { a.x } -> std::convertible_to<double>;
  { a.y } -> std::convertible_to<double>;
};

// Advanced Point Parser with template support and modern C++ features
template <PointType PointT = Point>
class PointParser {
 public:
  // Type aliases
  using PointVector = std::vector<PointT>;

  // Parse from a file with full error context
  static std::expected<PointVector, ParseError> ParseFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
      return std::unexpected(ParseError("Unable to open file", 0, 0, filename, "file_open"));
    }
    return ParseFromStream(file);
  }

  // Parse from an input stream with comprehensive error handling
  static std::expected<PointVector, ParseError> ParseFromStream(std::istream& input) {
    PointVector points;
    std::string line;
    int line_number = 1;
    std::string full_input_context;

    // Capture full input context for error reporting
    std::string input_context;
    {
      std::stringstream buffer;
      buffer << input.rdbuf();
      input_context = buffer.str();
      input.clear();
      input.seekg(0);
    }

    while (std::getline(input, line)) {
      // Trim leading and trailing whitespace
      line.erase(0, line.find_first_not_of(" \t"));
      line.erase(line.find_last_not_of(" \t") + 1);

      // Skip empty lines
      if (line.empty()) {
        line_number++;
        continue;
      }

      // try to parse amount of points
      if (line_number == 1) {
        int amount;
        try {
          amount = std::stoi(line);
        } catch (const std::exception& e) {
          return std::unexpected(ParseError(e.what(),
                                            line_number,
                                            0,  // Could be enhanced to track column
                                            line,
                                            line));
        }
        if (amount < 0) {
          return std::unexpected(ParseError("Invalid amount of points",
                                            line_number,
                                            0,  // Could be enhanced to track column
                                            line,
                                            line));
        }
        line_number++;
        continue;
      }

      try {
        auto point_result = ParseSinglePoint(line, line_number, input_context);
        if (point_result) {
          points.push_back(*point_result);
        } else {
          return std::unexpected(point_result.error());
        }
      } catch (const std::exception& e) {
        return std::unexpected(ParseError(e.what(),
                                          line_number,
                                          0,  // Could be enhanced to track column
                                          line,
                                          line));
      }
      line_number++;
    }

    if (points.size() != std::stoi(input_context)) {
      return std::unexpected(ParseError("Invalid amount of points",
                                        line_number,
                                        0,  // Could be enhanced to track column
                                        line,
                                        line));
    }

    return points;
  }

  // Ranges-based parsing (C++20 feature)
  template <std::ranges::input_range Range>
  static std::expected<PointVector, ParseError> ParseFromRange(Range&& range) {
    PointVector points;
    int line_number = 1;

    for (const auto& line : range) {
      try {
        auto point_result = ParseSinglePoint(line, line_number, "");
        if (!point_result) {
          return std::unexpected(point_result.error());
        }
        points.push_back(*point_result);
      } catch (const std::exception& e) {
        return std::unexpected(ParseError(e.what(), line_number, 0, line, line));
      }
      line_number++;
    }

    return points;
  }

 private:
  // Advanced single point parsing with detailed validation
  static std::expected<PointT, ParseError> ParseSinglePoint(const std::string& line,
                                                            int line_number,
                                                            const std::string& input_context) {
    std::istringstream ss(line);
    PointT point;

    // Use std::from_chars for precise numeric parsing (C++17)
    auto parse_coordinate = [&ss](double& coord) -> bool {
      std::string coord_str;
      if (!(ss >> coord_str))
        return false;

      double value;
      auto [ptr, ec] =
          std::from_chars(coord_str.data(), coord_str.data() + coord_str.size(), value);

      if (ec != std::errc())
        return false;
      coord = value;
      return true;
    };

    // Parse x coordinate
    if (!parse_coordinate(point.x)) {
      return std::unexpected(ParseError("Invalid x-coordinate", line_number, 1, line, line));
    }

    // Parse y coordinate
    if (!parse_coordinate(point.y)) {
      return std::unexpected(ParseError(
          "Invalid y-coordinate", line_number, line.find_first_of(" \t") + 1, line, line));
    }

    // Check for extra tokens
    std::string extra;
    if (ss >> extra) {
      return std::unexpected(
          ParseError("Extra tokens after coordinates", line_number, line.size(), line, extra));
    }

    return point;
  }
};

// Convenience functions
template <PointType PointT = Point>
inline auto ParsePointsFromFile(const std::string& filename) {
  return PointParser<PointT>::ParseFromFile(filename);
}

template <PointType PointT = Point>
inline auto ParsePointsFromString(const std::string& input_string) {
  std::istringstream ss(input_string);
  return PointParser<PointT>::ParseFromStream(ss);
}

}  // namespace cya