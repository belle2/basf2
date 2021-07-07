/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/HistoModule.h>

#include <string>
#include "TH1F.h"

namespace Belle2 {
  /**
   * This Module, made for ExpressReco, monitors the position
   * and the dimension of the beamspot using mu+mu- events
   */
  class IPDQMExpressRecoModule : public HistoModule {

  public:
    /**
     * Constructor
     */
    IPDQMExpressRecoModule();

    /**
     * Initialize the module
     */
    void initialize() override;

    /**
     * Called when entering a new run
     * Reset the histograms and counter m_r and clear the vectors
     */
    void beginRun() override;

    /**
     * Event processor
     * The main analysis happens here
     */
    void event() override;

    /**
     * End-of-run action.
     * The histograms range is set to be large 10 sigmas and centered on the mean
     */
    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

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
    /**Energy in LAB frame*/
    TH1F* m_h_E = nullptr;
    /** y pull*/
    TH1F* m_h_pull = nullptr;
    /** y resolution */
    TH1F* m_h_y_risol = nullptr;
    /** initial histogram for median calculation*/
    TH1F* m_h_temp = nullptr;
    /** xx coord*/
    TH1F* m_h_xx = nullptr;
    /** yy coord*/
    TH1F* m_h_yy = nullptr;
    /** zz coord*/
    TH1F* m_h_zz = nullptr;
    /** xz coord*/
    TH1F* m_h_xz = nullptr;
    /** yz coord*/
    TH1F* m_h_yz = nullptr;
    /** xy coord*/
    TH1F* m_h_xy = nullptr;

    /**Var x*/
    TH1F* m_h_cov_x_x = nullptr;
    /**Var y*/
    TH1F* m_h_cov_y_y = nullptr;
    /**Var z*/
    TH1F* m_h_cov_z_z = nullptr;
    /**Cov xz*/
    TH1F* m_h_cov_x_z = nullptr;
    /**Cov yz*/
    TH1F* m_h_cov_y_z = nullptr;
    /**Cov xy*/
    TH1F* m_h_cov_x_y = nullptr;
    /**store the y coordinates for the pull*/
    std::vector<float> m_v_y;
    /**store the y errors for the pull*/
    std::vector<float> m_err_y;

    Double_t m_median = 0; /**<The median of y coord*/
    Double_t m_quantile = 0.5;/**<The 0.5 quantile for the median*/
    Int_t m_r = 0; /**<Counter for sampling*/
    Int_t m_size = 200; /**<Size for sampling*/
    Int_t m_no_units = 5; /**<Number of units*/
    Int_t m_size_per_unit = m_size / m_no_units; /**<Size for sampling per each unit*/
    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

