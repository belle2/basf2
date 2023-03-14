/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : V0ObjectsDQMModule.h
// Description : Module to monitor displaced vertices
//-

#include <framework/core/HistoModule.h>
#include <string>
#include "TH2F.h"

namespace Belle2 {

  /** DQM modules to monitor V0 objects */
  class V0ObjectsDQMModule : public HistoModule {

  public:

    /**
     * Constructor.
     */
    V0ObjectsDQMModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;;

    /**
     * Definition of the histograms.
     */
    void defineHisto() override final;

  private:

    /** x vs. y in slices of z */
    TH2F* m_h_xvsy[32] = {nullptr};

    /** x vs. z*/
    TH2F* m_h_xvsz = nullptr;

    /** Name of the V0 particle list */
    std::string m_V0PListName = "";
  };

} // end namespace Belle2

