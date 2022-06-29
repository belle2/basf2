/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Belle 2 headers. */
#include <framework/core/HistoModule.h>

/* ROOT headers. */
#include <TH1F.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * This Module, made for monitors the position and the size
   * of the interaction point using mu+mu- events.
   */
  class IPDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    IPDQMModule();

    /**
     * Initialize the module
     */
    void initialize() override;

    /**
     * Called when entering a new run
     * Reset the histograms
     */
    void beginRun() override;

    /**
     * Event processor
     * The main analysis happens here
     */
    void event() override;

    /**
     * Defining the histograms.
     * Every new histogram added to the module has to be initialized here.
     */
    void defineHisto() override;

  private:

    /** x coord*/
    TH1F* m_h_x = nullptr;
    /** y coord*/
    TH1F* m_h_y = nullptr;
    /** z coord*/
    TH1F* m_h_z = nullptr;
    /** x coord momentum in LAB frame*/
    TH1F* m_h_px = nullptr;
    /** y coord momentum in LAB frame*/
    TH1F* m_h_py = nullptr;
    /** z coord momentum  in LAB frame*/
    TH1F* m_h_pz = nullptr;
    /** Energy in LAB frame*/
    TH1F* m_h_E = nullptr;
    /** Var x*/
    TH1F* m_h_cov_x_x = nullptr;
    /** Var y*/
    TH1F* m_h_cov_y_y = nullptr;
    /** Var z*/
    TH1F* m_h_cov_z_z = nullptr;
    /** Cov xz*/
    TH1F* m_h_cov_x_z = nullptr;
    /** Cov yz*/
    TH1F* m_h_cov_y_z = nullptr;
    /** Cov xy*/
    TH1F* m_h_cov_x_y = nullptr;
    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";
    /** Mode of online processing ("HLT" or "ExpressReco") */
    std::string m_onlineMode = "";

  };

} // end namespace Belle2

