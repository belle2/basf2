#ifndef HISTO_MODULE_H
#define HISTO_MODULE_H
//+
// File : HistoModule.h
// Description : Base class of module with histograms
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 30 - Jul - 2010
//-

#include <string>

#include "framework/core/Module.h"

namespace Belle2 {
  class HistoModule : public Module {
  public:
    // Constructor and Destructor
    HistoModule();
    virtual ~HistoModule();

    // Member functions (for event processing)
    virtual void initialize() {};
    virtual void beginRun() {};
    virtual void event() {};
    virtual void endRun() {};
    virtual void terminate() {};

    // function to define histograms
    virtual void defineHisto() {};
  };

}
#endif // P_EVENT_SERVER_H



