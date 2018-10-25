/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_OCCUPANCY_H_
#define SVD_OCCUPANCY_H_

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDHistograms.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/calibration/SVDNoiseCalibrations.h>


#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TList.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** The SVD OccupancyAnalysis Module
   *
   */

  class SVDOccupancyAnalysisModule : public Module {

  public:

    SVDOccupancyAnalysisModule();

    virtual ~SVDOccupancyAnalysisModule();
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_ShaperDigitName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    float m_group = 10000;
    float m_minZS = 3;
    float m_maxZS = 6;
    int m_pointsZS = 7;
    bool m_FADCmode;

  private:

    int m_nEvents;
    StoreArray<SVDShaperDigit> m_svdShapers;
    StoreObjPtr<EventMetaData> m_eventMetaData;

    SVDNoiseCalibrations m_NoiseCal;


    //SHAPER
    SVDHistograms<TH1F>* m_histo_occ;
    SVDHistograms<TH1F>* m_histo_zsOcc;
    SVDHistograms<TH1F>* m_histo_zsOccSQ;
    SVDHistograms<TH2F>* m_histo_occtdep;

  };
}

#endif /* SVDOccupancyAnalysisModule_H_ */

