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
    std::string m_rootFileName = "";   /**< root file name */
    std::string m_ShaperDigitName = "SVDShaperDigits";   /**< ShaperDigit StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

    float m_group = 10000; /**<number of events to comput occupancy for occ VS time*/
    float m_minZS = 3; /**<minimum zero suppresion cut*/
    float m_maxZS = 6; /**max zero suppression cut*/
    int m_pointsZS = 7; /**<num,ner of steps for different ZS cuts*/
    bool m_FADCmode = true; /**if true, ZS done with same algorithm as on FADC*/

  private:

    int m_nEvents = 0; /**< number of events*/
    StoreArray<SVDShaperDigit> m_svdShapers; /**<SVDShaperDigit StoreArray*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**<Event Meta Data StoreObjectPointer*/

    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibrations db object*/


    //SHAPER
    SVDHistograms<TH1F>* m_histo_occ = nullptr; /**<occupancy histograms*/
    SVDHistograms<TH1F>* m_histo_zsOcc = nullptr; /**<occupancy VS ZScut histograms*/
    SVDHistograms<TH1F>* m_histo_zsOccSQ = nullptr; /**< occupancy VS ZS cut swuared histograms*/
    SVDHistograms<TH2F>* m_histo_occtdep = nullptr; /**< occupancy VS event number*/

  };
}

#endif /* SVDOccupancyAnalysisModule_H_ */

