// Verilated Library
#include "verilated.h"
// Catch Library
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main( int argc, char* argv[] ) {
  // global setup...
  Verilated::commandArgs(argc, argv); // call Verilated Commands
  
  int result = Catch::Session().run( argc, argv );

  // global clean-up...

  return result;
}
