#ifndef _Belle2_HSLB_hh
#define _Belle2_HSLB_hh

#include "Module.h"

namespace Belle2 {

  class FEEModule;

  class HSLB : public Module {

  public:
    HSLB() : _used(true), _firmware(""),
      _board(NULL) {}
    virtual ~HSLB() throw() {}

  public:
    bool isUsed() const { return _used; }
    int getTriggerMode() const { return _trg_mode; }
    const std::string& getFirmware() const { return _firmware; }
    FEEModule* getFEEModule() const { return _board; }
    void setUsed(bool used) { _used = used; }
    void setTriggerMode(int mode) { _trg_mode = mode; }
    void setFirmware(const std::string& firmware) { _firmware = firmware; }
    void setFEEModule(FEEModule* board) { _board = board; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    bool _used;
    int _trg_mode;
    std::string _firmware;
    FEEModule* _board;

  };

}

#endif
