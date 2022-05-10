/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : PysicsObjectsDQMModule.h
// Description : Module to monitor physics objects
//-

#include <framework/core/HistoModule.h>

#include <string>
#include "TH1F.h"

namespace Belle2 {

  /**
   * Physics objects DQM module.
   */
  class PhysicsObjectsDQMModule : public HistoModule {

  public:

    /**
     * Constructor.
     */
    PhysicsObjectsDQMModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

    /**
     * Definition of the histograms.
     */
    void defineHisto() override;

  private:
    /** KS0 invariant mass */
    TH1F* m_h_mKS0 = nullptr;

    /** PI0 invariant mass */
    TH1F* m_h_mPI0 = nullptr;

    /** Ups invariant mass */
    TH1F* m_h_mUPS = nullptr;

    /** R2 */
    TH1F* m_h_R2 = nullptr;

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    /** Trigger identifier string used to select events for the mumu histograms */
    std::string m_triggerIdentifierMuMu = "";

    /** Name of the pi0 particle list */
    std::string m_pi0PListName = "";

    /** Name of the KS0 particle list */
    std::string m_ks0PListName = "";

    /** Name of the Ups particle list */
    std::string m_upsPListName = "";
  };

} // end namespace Belle2

