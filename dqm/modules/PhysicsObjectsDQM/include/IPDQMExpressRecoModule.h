/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2017 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Gaetano de Marino                                         *
 *                                                                         *
 *                                                                         *
 * This software is provided "as is" without any warranty.                 *
 ***************************************************************************/
#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>
#include "TH1F.h"
#include "TPaveStats.h"

namespace Belle2 {
  /*
    This Module, made for ExpressReco, monitors the position
    and the dimension of the beamspot using mu+mu- events
   */
  class IPDQMExpressRecoModule : public HistoModule {

  public:

    IPDQMExpressRecoModule();

    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;
    void defineHisto() override;


  private:

    /** x coord*/
    TH1F* m_h_x = nullptr;
    /** y coord*/
    TH1F* m_h_y = nullptr;
    /** z coord*/
    TH1F* m_h_z = nullptr;
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

    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

