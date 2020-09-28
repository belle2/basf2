#include "daq/slc/version/Version.h"

//define if DEF_DAQ_SLC_VERSION is not set at compilation using `g++ -DDEF_DAQ_SLC_VERSION=\"version_whatever\"
#ifndef DEF_DAQ_SLC_VERSION
//use default "NOT_SET"
#define DEF_DAQ_SLC_VERSION "NOT_SET"
#endif

const std::string DAQ_SLC_VERSION::DAQ_SLC_VERSION(DEF_DAQ_SLC_VERSION);

