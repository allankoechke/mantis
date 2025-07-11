#include <cstdio>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <cerrno>
#include <sstream>
#include <iostream>
#include <cctype>  // For tolower() and toupper()

#include "./shunting-yard.h"
#include "./shunting-yard-exceptions.h"

using namespace cparse;

/* * * * * Built-in Features: * * * * */

#include "./builtin-features/functions.inc"

#include "./builtin-features/operations.inc"

#include "./builtin-features/reservedWords.inc"

#include "./builtin-features/typeSpecificFunctions.inc"

inline void cparse_startup() {
  builtin_functions::Startup();
  builtin_operations::Startup();
  builtin_reservedWords::Startup();
  builtin_typeSpecificFunctions::Startup();
}
