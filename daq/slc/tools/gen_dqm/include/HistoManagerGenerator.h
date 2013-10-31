#ifndef _B2DQM_HistoManagerGenerator_hh
#define _B2DQM_HistoManagerGenerator_hh

#include <string>

namespace Belle2 {

  class HistoManagerGenerator {

  public:
    HistoManagerGenerator(const std::string name,
                          const std::string& output_dir) {
      _name = name;
      _output_dir = output_dir;
    }
    virtual ~HistoManagerGenerator() throw() {}

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
