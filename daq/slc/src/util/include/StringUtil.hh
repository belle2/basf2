#ifndef _B2DAQ_StringUtil_hh
#define _B2DAQ_StringUtil_hh

#include <string>
#include <vector>

namespace B2DAQ {

  typedef std::vector<std::string> StringList;

  StringList split(const std::string& str, 
		   const char type, size_t max=0);
  std::string replace(const std::string & source,
		      const std::string & pattern,
		      const std::string & placement);
  std::string form(const std::string& string, ...);
  std::string toupper(const std::string& str);
  std::string tolower(const std::string& str);

};

#endif
