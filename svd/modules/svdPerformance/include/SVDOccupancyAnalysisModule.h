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
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_ShaperDigitName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    float m_group = 10000;
    float m_minZS = 3;
    float m_maxZS = 6;
    int m_pointsZS = 7;

  private:

    int m_nEvents;
    StoreArray<SVDShaperDigit> m_svdShapers;
    StoreObjPtr<EventMetaData> m_eventMetaData;

    SVDNoiseCalibrations m_NoiseCal;

    static const int m_nLayers = 4;
    static const int m_nSensors = 5;
    static const int m_nSides = 2;

    unsigned int sensorsOnLayer[4];

    TList* m_histoList_shaper[m_nLayers];

    //SHAPER
    TH1F* h_occ[m_nLayers][m_nSensors][m_nSides]; //number per event
    TH1F* h_zsOcc[m_nLayers][m_nSensors][m_nSides]; //number per event
    TH1F* h_zsOccSQ[m_nLayers][m_nSensors][m_nSides]; //number per event

    TH2F* h_occtdep[m_nLayers][m_nSensors][m_nSides]; //number per event
    int getSensor(int sensor)
    {
      return sensor - 1;;
    }

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */
    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = NULL);  /**< thf */


  };
}

#endif /* SVDOccupancyAnalysisModule_H_ */

