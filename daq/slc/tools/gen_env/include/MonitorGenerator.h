#ifndef _Belle2_MonitorGenerator_h
#define _Belle2_MonitorGenerator_h

#include <string>

namespace Belle2 {

  class MonitorGenerator {

  public:
    MonitorGenerator(const std::string name,
                     const std::string& output_dir) {
      _name = name;
      _output_dir = output_dir;
    }
    virtual ~MonitorGenerator() throw() {}

  public:
    void create();

  private:
    std::string createHeader();
    std::string createSource();
    std::string createLibSource();

  private:
    std::string _name;
    std::string _output_dir;

  };

}

#endif
