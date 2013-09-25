#ifndef _B2DAQ_COPPERCodeGenerator_hh
#define _B2DAQ_COPPERCodeGenerator_hh

#include <string>
#include <map>

namespace B2DAQ {

  class COPPERNode;

  class COPPERCodeGenerator {

  public:
    COPPERCodeGenerator(const std::string& output_dir) {
      _output_dir = output_dir;
    }
    virtual ~COPPERCodeGenerator() throw() {}

  public:
    void setCOPPERs(const std::map<std::string, COPPERNode*>& copper_m) {
      _copper_m = copper_m;
    }

  private:
    void setCOPPER(COPPERNode* copper) { _copper = copper; }
    void setModuleClass(const std::string& module_class) {
      _module_class = module_class;
    }

  public:
    void create();
    std::string createDataHeader();
    std::string createDataHandlerHeader();
    std::string createDataHandlerSource();
    std::string createDynamicLoadSource();
    
  private:
    std::map<std::string, COPPERNode*> _copper_m;
    COPPERNode* _copper;
    std::string _module_class;
    std::string _output_dir;

  };

}

#endif
