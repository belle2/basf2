//
// nsmget.h
//
// original author: Thorsten Roeder (MPI/Munich)
// modifications:   Mikihiko Nakao (KEK)
//
// 20140920 initial version named as b2nsmaccesslib.hpp
// 20140922 file/function/type names and style modified by MN
//

#ifndef __nsmget_h__
#define __nsmget_h__

#include <string>
#include <map>

#include "../corelib/nsmparse.h"

enum nsmget_native_t {
  INT64  = 0,
  INT32  = 1,
  INT16  = 2,
  CHAR   = 3,
  UINT64 = 4,
  UINT32 = 5,
  UINT16 = 6,
  UCHAR  = 7,
  DOUBLE = 8,
  FLOAT  = 9
};

struct nsmget_entry_t {
  std::size_t offset;         //!< memory offset (in bytes) within struct
  const NSMparse* parse_entry;  //!< single raw entry of the parse result
  nsmget_native_t type;
};

typedef std::map<std::string, nsmget_entry_t> nsmget_map_t;

NSMparse*       nsmget_file(const std::string& format_name);
// after nsm2-1944
int             nsmget_path(const std::string& variable_path,
                            std::string& data_name,
                            std::string& variable_name);
// until nsm2-1941
int             nsmget_path(const std::string& variable_path,
                            std::string& data_name,
                            std::string& struct_name,
                            std::string& variable_name);
const NSMparse* nsmget_map(const NSMparse* start,
                           const std::string prefix,
                           std::size_t& current_offset,
                           std::map<std::string, nsmget_entry_t>& mapping);
nsmget_native_t nsmget_type(const char c);

#endif // __nsmget_h__
