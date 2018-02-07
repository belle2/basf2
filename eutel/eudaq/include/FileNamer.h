#ifndef EUDAQ_INCLUDED_FileNamer
#define EUDAQ_INCLUDED_FileNamer

#include <eutel/eudaq/Utils.h>
#include <string>
#include <vector>

namespace eudaq {

  class FileNamer {
  public:
    FileNamer(const std::string& pattern = default_pattern);
    FileNamer& SetReplace(char opt, const std::string& val);
    template <typename T>
    FileNamer& Set(char opt, const T& val);
    operator std::string() const;
    static const std::string default_pattern;
  private:
    struct part_t {
      part_t(char n, int l = 0) : name(n), len(l) {}
      part_t(const std::string& s) : name(0), len(0), str(s) {}
      char name;
      int len;
      std::string str;
    };
    std::vector<part_t> m_parts;
  };

  template <typename T>
  FileNamer& FileNamer::Set(char opt, const T& val)
  {
    for (size_t i = 0; i < m_parts.size(); ++i) {
      if (m_parts[i].name == opt) {
        m_parts[i].str = to_string(val, m_parts[i].len);
      }
    }
    return *this;
  }

}

#endif // EUDAQ_INCLUDED_FileNamer
