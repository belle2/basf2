#include <eutel/eudaq/FileNamer.h>
#include <eutel/eudaq/Exception.h>

#include <iostream>

namespace eudaq {

  const std::string FileNamer::default_pattern = "../data/run$6R$X";

  FileNamer::FileNamer(const std::string& p)
  {
    std::string pattern(p == "" ? default_pattern : p);
    size_t i0 = 0;
    //std::cout << "FileNamer " << pattern << std::endl;
    for (;;) {
      size_t i1 = pattern.find('$', i0);
      //std::cout << "loop i0 " << i0 << ", i1 " << i1 << std::endl;
      if (i1 != i0) {
        if (i1 == std::string::npos) {
          m_parts.push_back(part_t(pattern.substr(i0)));
        } else {
          m_parts.push_back(part_t(pattern.substr(i0, i1 - i0)));
        }
        //std::cout << "part " << m_parts.back().str << std::endl;
      }
      if (i1 == std::string::npos) {
        //std::cout << "done" << std::endl;
        break;
      }
      i1 += 1;
      if (i1 >= pattern.length()) EUDAQ_THROW("Bad file pattern: '" + pattern + "'");
      if (pattern[i1] == '-' || pattern[i1] == '+') i1 += 1;
      size_t i2 = pattern.find_first_not_of("0123456789", i1);
      //std::cout << "i1 " << i1 << ", i2 " << i2 << std::endl;
      int num = 0;
      if (i2 != i1) {
        num = from_string(pattern.substr(i1 - (pattern[i1 - 1] == '-'), i2 - i1), 0);
      }
      m_parts.push_back(part_t(pattern[i2], num));
      //std::cout << "part " << m_parts.back().name << ", " << m_parts.back().len << std::endl;
      i0 = i2 + 1;
      if (i0 == pattern.length()) break;
    }
  }

  FileNamer& FileNamer::SetReplace(char opt, const std::string& val)
  {
    if (val.find_first_not_of("0123456789") == std::string::npos) {
      Set(opt, from_string(val, 0ULL));
    } else {
      m_parts.clear();
      m_parts.push_back(part_t(val));
    }
    return *this;
  }

  FileNamer::operator std::string() const
  {
    std::string result;
    for (size_t i = 0; i < m_parts.size(); ++i) {
      result += m_parts[i].str;
    }
    //std::cout << "FileNamer " << result << std::endl;
    return result;
  }

}
