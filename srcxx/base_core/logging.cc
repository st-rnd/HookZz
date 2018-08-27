#include "base_core/logging.h"

#include "base_core/platform/platform.h"

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace zz {
namespace base {} // namespace base
} // namespace zz

void Fatal(const char *file, int line, const char *format, ...) {
  // Print the formatted message to stdout without cropping the output.
  zz::base::OS::PrintError("\n\n#\n# Fatal error in %s, line %d\n# ", file, line);

  // Print the error message.
  va_start(arguments, format);
  zz::base::OS::VPrintError(format, arguments);
  va_end(arguments);

  fflush(stderr);
}