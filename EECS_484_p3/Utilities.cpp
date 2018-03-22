#include "Utilities.h"                                  // file-specific header
#include <cassert>                                      // for static_assert


const char* kPrintPrefix = "@@@     ";
const int kIndentIncr = 4;

static_assert(kLeafOrder >= 4, "The order of leaf nodes must be at least 1");
static_assert(kInnerOrder >= 4, "The order of inner nodes must be at least 1");
