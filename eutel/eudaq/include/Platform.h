#ifndef EUDAQ_INCLUDED_Platform
#define EUDAQ_INCLUDED_Platform

#ifndef EUDAQ_PLATFORM
# define EUDAQ_PLATFORM PF_LINUX
#endif

#define PF_LINUX  1
#define PF_MACOSX 2
#define PF_CYGWIN 3
#define PF_WIN32  4

#define EUDAQ_PLATFORM_IS(P) (EUDAQ_PLATFORM == PF_ ## P)

#endif // EUDAQ_INCLUDED_Platform
