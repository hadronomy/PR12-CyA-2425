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
 * Archivo main.cc: client program.
 * Referencias:
 */

#include <exception>
#include <iostream>

#include "cya/program.h"

int main(const int argc, const char* argv[]) {
  try {
    cya::Program program(argc, argv);
    program.Run();
  } catch (const std::exception& exception) {
    std::cerr << exception.what() << std::endl;
  }
  return 0;
}
