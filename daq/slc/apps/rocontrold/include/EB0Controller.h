#ifndef _Belle2_EB0Controller_h
#define _Belle2_EB0Controller_h

#include "daq/slc/apps/rocontrold/ROController.h"
#include "daq/slc/readout/mmap_statistics.h"

namespace Belle2 {

  class EB0Controller : public ROController {

  public:
    EB0Controller(): m_eb_stat(NULL) {}
    virtual ~EB0Controller() throw();

  public:
    virtual void check();

  protected:
    virtual void initArguments(const DBObject& obj);
    virtual bool loadArguments(const DBObject& obj);

  private:
    void readDB(const DBObject& obj, int& port,
                std::string& executable, bool& used, int& nsenders);

  private:
    eb_statistics* m_eb_stat;
    int m_nsenders;
    double m_nevent_in[10];
    unsigned long long m_total_byte_in[10];
    double m_nevent_out[10];
    unsigned long long m_total_byte_out[10];
    double m_t0;

  };

}

#endif
