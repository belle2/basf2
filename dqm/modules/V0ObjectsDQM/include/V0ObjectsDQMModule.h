//+
// File : V0ObjectsDQMModule.h
// Description : Module to monitor displaced vertices
//
// Author : Bryan Fulsom PNNL
// Date : 2019-01-17
//-

#include <framework/core/HistoModule.h>
#include <string>
#include "TH2F.h"

namespace Belle2 {

  class V0ObjectsDQMModule : public HistoModule {

  public:

    V0ObjectsDQMModule();

  private:

    void initialize() override final;
    void beginRun() override final;
    void event() override final;;

    void defineHisto() override final;

    /* x vs. y in slices of z */
    TH2F* m_h_xvsy[32] = {nullptr};

    /** Name of the V0 particle list */
    std::string m_V0PListName = "";
  };

} // end namespace Belle2

