#ifndef Belle2_ECLShaperConfig_h
#define Belle2_ECLShaperConfig_h

#include <map>
#include <string>

namespace Belle2 {

  class ECLShaperRegister {

  public:
    ECLShaperRegister() {}
    ECLShaperRegister(int adr, int val, const char* name) {
      this->adr = adr;
      this->val = val;
      this->name = name;
    }

  public:
    int adr;
    int val;
    std::string name;

  };

  typedef std::map<std::string, ECLShaperRegister> ECLShaperRegisterList;
  typedef std::map<int, ECLShaperRegisterList> ECLShaperConfig;

}

#endif
