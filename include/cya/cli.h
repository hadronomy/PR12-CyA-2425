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
 * Archivo cli.h: Implementación de un parser de línea de comandos
 * Referencias:
 */

#pragma once

#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace cli {

namespace colors {
constexpr char RESET[] = "\033[0m";
constexpr char BOLD[] = "\033[1m";
constexpr char RED[] = "\033[31m";
constexpr char GREEN[] = "\033[32m";
constexpr char YELLOW[] = "\033[33m";
constexpr char CYAN[] = "\033[36m";
}  // namespace colors

class CliParserError : public std::runtime_error {
 public:
  explicit CliParserError(const std::string& message) : std::runtime_error(message) {}
};

class ArgumentParser {
 public:
  explicit ArgumentParser(std::string program_name, std::string program_description = "")
      : program_name_(std::move(program_name)),
        program_description_(std::move(program_description)) {
    AddArgument("help", "h", "Show this help message and exit").SetFlag().SetDefaultValue(false);
  }

  class Argument {
   public:
    Argument() = default;

    Argument(std::string name, std::string short_name, std::string description,
             ArgumentParser* parser)
        : name_(std::move(name)),
          short_name_(std::move(short_name)),
          description_(std::move(description)),
          parser_(parser) {}

    Argument& Required() {
      is_required_ = true;
      return *this;
    }

    template <typename T>
    Argument& SetDefaultValue(T value) {
      default_value_ = std::move(value);
      current_value_ = value;
      return *this;
    }

    Argument& SetMultiple(size_t count = 0) {
      is_multiple_ = true;
      value_count_ = count;  // 0 means any number of values
      return *this;
    }

    Argument& Validate(std::function<bool(const std::string&)> validator) {
      validator_ = std::move(validator);
      return *this;
    }

    Argument& SetFlag() {
      is_flag_ = true;
      return *this;
    }

    ArgumentParser& End() { return *parser_; }

    bool WasPassed() const { return was_passed_; }

   private:
    friend class ArgumentParser;

    std::string name_;
    std::string short_name_;
    std::string description_;
    bool is_required_ = false;
    bool is_flag_ = false;
    bool is_multiple_ = false;
    size_t value_count_ = 0;  // 0 = any number, >0 = fixed number
    std::any default_value_;
    std::any current_value_;
    std::vector<std::string> multiple_values_;  // Stores list of values
    std::function<bool(const std::string&)> validator_;
    ArgumentParser* parser_ = nullptr;
    bool was_passed_ = false;  // Tracks if the argument was explicitly passed
  };

  class PositionalArgument {
   public:
    PositionalArgument(std::string name, std::string description, ArgumentParser* parser)
        : name_(std::move(name)), description_(std::move(description)), parser_(parser) {}

    PositionalArgument& Required() {
      is_required_ = true;
      return *this;
    }

    template <typename T>
    PositionalArgument& SetDefaultValue(T value) {
      default_value_ = std::move(value);
      current_value_ = value;
      return *this;
    }

    ArgumentParser& End() { return *parser_; }

   private:
    friend class ArgumentParser;

    std::string name_;
    std::string description_;
    bool is_required_ = false;
    std::any default_value_;
    std::any current_value_;
    ArgumentParser* parser_ = nullptr;
  };

  Argument& AddArgument(const std::string& name, const std::string& short_name = "",
                        const std::string& description = "") {
    arguments_[name] = Argument(name, short_name, description, this);
    return arguments_.at(name);
  }

  PositionalArgument& AddPositionalArgument(const std::string& name,
                                            const std::string& description = "") {
    positional_arguments_.emplace_back(name, description, this);
    return positional_arguments_.back();
  }

  void Parse(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    Parse(args);
  }

  void Parse(const std::vector<std::string>& args) {
    parsed_positional_arguments_.clear();
    help_requested_ = false;

    size_t positional_index = 0;

    for (size_t i = 0; i < args.size(); ++i) {
      const std::string& arg = args[i];

      if (arg == "--help" || arg == "-h") {
        PrintHelp();
        help_requested_ = true;
        return;
      }

      if (arg.starts_with("--")) {
        ParseLongArgument(arg.substr(2), args, i);
      } else if (arg.starts_with("-")) {
        ParseShortArgument(arg.substr(1), args, i);
      } else {
        // Positional arguments are handled here
        if (positional_index >= positional_arguments_.size()) {
          PrintError("Unexpected positional argument: " + arg);
          throw CliParserError("Unexpected positional argument: " + arg);
        }
        ParsePositionalArgument(positional_arguments_[positional_index], arg);
        ++positional_index;
      }
    }

    ValidateArguments();
  }

  template <typename T>
  T GetValue(const std::string& name) const {
    // Check optional arguments
    auto it = arguments_.find(name);
    if (it != arguments_.end()) {
      const Argument& arg = it->second;

      if (arg.is_multiple_) {
        if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          return arg.multiple_values_;
        } else {
          throw CliParserError("Type mismatch: argument '" + name + "' expects a list of values.");
        }
      } else if (arg.current_value_.has_value()) {
        return std::any_cast<T>(arg.current_value_);
      }

      throw CliParserError("No value found for argument: --" + name);
    }

    // Check positional arguments
    for (const auto& pos_arg : positional_arguments_) {
      if (pos_arg.name_ == name) {
        if (pos_arg.current_value_.has_value()) {
          return std::any_cast<T>(pos_arg.current_value_);
        }
        throw CliParserError("No value found for positional argument: " + name);
      }
    }

    throw CliParserError("Argument not found: " + name);
  }

  bool IsHelpRequested() const { return help_requested_; }

  bool WasArgumentPassed(const std::string& name) const {
    auto it = arguments_.find(name);
    if (it != arguments_.end()) {
      return it->second.WasPassed();
    }
    throw CliParserError("Argument not found: --" + name);
  }


 private:
  void PrintHelp() const {
    std::cout << colors::BOLD << "Usage: " << colors::RESET << program_name_ << " ";
    if (!arguments_.empty())
      std::cout << "[OPTIONS] ";
    for (const auto& pos_arg : positional_arguments_) {
      std::cout << colors::BOLD << "<" << pos_arg.name_ << ">" << colors::RESET << " ";
    }
    std::cout << "\n\n";

    if (!program_description_.empty()) {
      std::cout << colors::BOLD << "Description:\n"
                << colors::RESET << "  " << program_description_ << "\n\n";
    }

    if (!arguments_.empty()) {
      std::cout << colors::BOLD << "Options:\n" << colors::RESET;
      for (const auto& [name, arg] : arguments_) {
        std::cout << "  " << colors::CYAN << "--" << name << colors::RESET;
        if (!arg.short_name_.empty()) {
          std::cout << ", " << colors::CYAN << "-" << arg.short_name_ << colors::RESET;
        }
        std::cout << "\n    " << arg.description_;
        if (arg.default_value_.has_value()) {
          std::cout << colors::YELLOW << " [default: " << GetDefaultValueString(arg) << "]"
                    << colors::RESET;
        }
        if (arg.is_required_) {
          std::cout << colors::RED << " (required)" << colors::RESET;
        }
        std::cout << "\n";
      }
      std::cout << "\n";
    }

    if (!positional_arguments_.empty()) {
      std::cout << colors::BOLD << "Positional Arguments:\n" << colors::RESET;
      for (const auto& pos_arg : positional_arguments_) {
        std::cout << "  " << colors::CYAN << pos_arg.name_ << colors::RESET;
        std::cout << "\n    " << pos_arg.description_;
        if (pos_arg.default_value_.has_value()) {
          std::cout << colors::YELLOW << " [default: " << GetDefaultValueString(pos_arg) << "]"
                    << colors::RESET;
        }
        if (pos_arg.is_required_) {
          std::cout << colors::RED << " (required)" << colors::RESET;
        }
        std::cout << "\n";
      }
      std::cout << "\n";
    }

    std::cout << colors::BOLD << "For more information, use --help or -h.\n" << colors::RESET;
  }

  void PrintError(const std::string& message, const std::string& suggestion = "") const {
    std::cerr << colors::BOLD << colors::RED << "Error: " << colors::RESET << message << "\n";

    if (!suggestion.empty()) {
      std::cerr << colors::BOLD << colors::YELLOW << "Hint: " << colors::RESET << suggestion
                << "\n";
    }

    std::cerr << "\n"
              << colors::BOLD << "Usage:\n"
              << colors::RESET << "  " << program_name_ << " [OPTIONS] ";
    for (const auto& pos_arg : positional_arguments_) {
      std::cerr << colors::BOLD << "<" << pos_arg.name_ << ">" << colors::RESET << " ";
    }
    std::cerr << "\n\n";

    std::cerr << colors::BOLD << "Run with --help or -h for detailed usage.\n" << colors::RESET;
  }

  std::string GetDefaultValueString(const Argument& arg) const {
    if (!arg.default_value_.has_value())
      return "N/A";

    try {
      if (arg.default_value_.type() == typeid(bool)) {
        return std::any_cast<bool>(arg.default_value_) ? "true" : "false";
      }
      if (arg.default_value_.type() == typeid(int)) {
        return std::to_string(std::any_cast<int>(arg.default_value_));
      }
      if (arg.default_value_.type() == typeid(std::string)) {
        return std::any_cast<std::string>(arg.default_value_);
      }
    } catch (const std::bad_any_cast&) {
      return "Unknown";
    }
    return "Unknown";
  }

  std::string GetDefaultValueString(const PositionalArgument& arg) const {
    if (!arg.default_value_.has_value())
      return "N/A";

    try {
      if (arg.default_value_.type() == typeid(bool)) {
        return std::any_cast<bool>(arg.default_value_) ? "true" : "false";
      }
      if (arg.default_value_.type() == typeid(int)) {
        return std::to_string(std::any_cast<int>(arg.default_value_));
      }
      if (arg.default_value_.type() == typeid(std::string)) {
        return std::any_cast<std::string>(arg.default_value_);
      }
    } catch (const std::bad_any_cast&) {
      return "Unknown";
    }
    return "Unknown";
  }

  void ParseLongArgument(const std::string& arg_name, const std::vector<std::string>& args,
                         size_t& current_index) {
    auto it = arguments_.find(arg_name);
    if (it == arguments_.end()) {
      PrintError("Unknown argument: --" + arg_name);
      throw CliParserError("Unknown argument: --" + arg_name);
    }

    Argument& argument = it->second;
    argument.was_passed_ = true;  // Mark as explicitly passed

    if (argument.is_flag_) {
      argument.current_value_ = true;
      return;
    }

    if (argument.is_multiple_) {
      argument.multiple_values_.clear();
      size_t values_to_parse =
          argument.value_count_ > 0 ? argument.value_count_ : args.size() - current_index - 1;

      for (size_t i = 0; i < values_to_parse; ++i) {
        if (current_index + 1 >= args.size()) {
          if (argument.value_count_ > 0) {
            PrintError("Insufficient values for argument: --" + arg_name);
            throw CliParserError("Insufficient values for argument: --" + arg_name);
          }
          break;
        }

        const std::string& next_value = args[current_index + 1];
        if (next_value.starts_with("--") ||
            (next_value.starts_with("-") && !std::isdigit(next_value[1]))) {
          if (argument.value_count_ > 0) {
            PrintError("Insufficient values for argument: --" + arg_name);
            throw CliParserError("Insufficient values for argument: --" + arg_name);
          }
          break;
        }

        argument.multiple_values_.push_back(next_value);
        ++current_index;
      }

      if (argument.value_count_ > 0 && argument.multiple_values_.size() != argument.value_count_) {
        PrintError("Expected " + std::to_string(argument.value_count_) +
                   " values for argument: --" + arg_name);
        throw CliParserError("Expected " + std::to_string(argument.value_count_) +
                             " values for argument: --" + arg_name);
      }

      argument.current_value_ = argument.multiple_values_;
    } else {
      if (current_index + 1 >= args.size() || args[current_index + 1].starts_with("-")) {
        PrintError("Missing value for argument: --" + arg_name);
        throw CliParserError("Missing value for argument: --" + arg_name);
      }
      argument.current_value_ = args[++current_index];
    }
  }

  void ParseShortArgument(const std::string& short_args, const std::vector<std::string>& args,
                          size_t& current_index) {
    for (char short_arg : short_args) {
      for (auto& [name, argument] : arguments_) {
        if (!argument.short_name_.empty() && argument.short_name_[0] == short_arg) {
          argument.was_passed_ = true;  // Mark as explicitly passed
          ParseLongArgument(name, args, current_index);
          break;
        }
      }
    }
  }

  void ParsePositionalArgument(PositionalArgument& pos_arg, const std::string& value) {
    pos_arg.current_value_ = value;
    parsed_positional_arguments_.push_back(value);
  }

  void ValidateArguments() {
    for (const auto& [name, arg] : arguments_) {
      if (arg.is_required_ && !arg.current_value_.has_value()) {
        PrintError("Missing required argument: --" + name);
        throw CliParserError("Missing required argument: --" + name);
      }
    }

    for (const auto& pos_arg : positional_arguments_) {
      if (pos_arg.is_required_ && !pos_arg.current_value_.has_value()) {
        PrintError("Missing required positional argument: " + pos_arg.name_);
        throw CliParserError("Missing required positional argument: " + pos_arg.name_);
      }
    }
  }

  std::string program_name_;
  std::string program_description_;
  std::map<std::string, Argument> arguments_;
  std::vector<PositionalArgument> positional_arguments_;
  std::vector<std::string> parsed_positional_arguments_;
  bool help_requested_ = false;
};

}  // namespace cli
