/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: casarosa                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDLATENCYCALIBRATIONMODULE_H
#define SVDLATENCYCALIBRATIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDShaperDigit.h>

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TList.h>

// forward declarations
class TTree;
class TFile;


namespace Belle2 {
  /**
   * this module perfoms an analysis to find the APV25 latency
   *
   */
  class SVDLatencyCalibrationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDLatencyCalibrationModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void event();

    /**  */
    virtual void terminate();

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    TH1F* h_maxAmplitudeU; /**< histogram containing the maximum of the 6 samples of the U SVDShaperDigit*/
    TH1F* h_maxAmplitudeV; /**< histogram containing the maximum of the 6 samples of the V SVDShaperDigit*/
    TH1F* h_ladder;

    TH1F* h_maxAmplitudeU_L3fw;
    TH1F* h_maxAmplitudeV_L3fw;
    TH1F* h_maxAmplitudeU_L3bw;
    TH1F* h_maxAmplitudeV_L3bw;
    TH1F* h_maxAmplitudeU_L4bw;
    TH1F* h_maxAmplitudeV_L4bw;
    TH1F* h_maxAmplitudeU_L5bw;
    TH1F* h_maxAmplitudeV_L5bw;
    TH1F* h_maxAmplitudeU_L6bw;
    TH1F* h_maxAmplitudeV_L6bw;

    TList* m_histoList;

  private:

    std::string m_shapersListName; /**< shapers list name */
    StoreArray<SVDShaperDigit> m_digits; /**< SVD digits*/

    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList);

  };
}

#endif /* SVDLATENCYCALIBRATIONMODULE_H */
