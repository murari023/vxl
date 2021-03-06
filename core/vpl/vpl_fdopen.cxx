// This is core/vpl/vpl_fdopen.cxx
#include <cstdio>
#include "vpl_fdopen.h"
#include <vcl_compiler.h>

std::FILE *vpl_fdopen(int fd, char const *mode)
{
#if defined(VCL_WIN32) && !defined(__CYGWIN__)
  return _fdopen(fd, mode);
#else
  return fdopen(fd, mode);
#endif
}
