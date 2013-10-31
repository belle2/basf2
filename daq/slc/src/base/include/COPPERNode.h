#ifndef _Belle2_COPPERNode_hh
#define _Belle2_COPPERNode_hh

#include "NSMNode.h"

namespace Belle2 {

  class DataSender;
  class HSLB;

  class COPPERNode : public NSMNode {

  public:
    static const size_t MAX_HSLBS = 4;

  public:
    COPPERNode(const std::string& name = "");
    virtual ~COPPERNode() throw();

  public:
    const DataSender* getSender() const throw() { return _sender; }
    DataSender* getSender() throw() { return _sender; }
    const HSLB* getHSLB(int slot) const throw() { return _hslb_v[slot]; }
    HSLB* getHSLB(int slot) throw() { return _hslb_v[slot]; }
    const std::string& getModuleClass() throw() { return _module_class; }
    void setHSLB(int slot, HSLB* hslb) throw() { _hslb_v[slot] = hslb; }
    void setModuleClass(const std::string& module_class) {
      _module_class = module_class;
    }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();
    virtual int getParams(const Command& command, unsigned int* pars,
                          std::string& datap);
    virtual void setParams(const Command& command, int npar,
                           const unsigned int* pars, const std::string& datap);

  private:
    DataSender* _sender;
    HSLB* _hslb_v[MAX_HSLBS];
    std::string _module_class;

  };

}

#endif
