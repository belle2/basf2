/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DATCONTracking)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DATCONTrackingModule::DATCONTrackingModule() : Module()
{
  //Set module properties
  setDescription("DATCON: Data Acquisition Tracking Concentrator Online Node \n"
                 "Algorithm for online data reduction in the PXD. \n"
                 "DATCON takes SVD hits as input and performs track reconstruction based on these data. \n"
                 "The tracks are extrapolated from the origin in the x-y-plane to the PXD sensors. \n"
                 "Around each extrapolated hit (called Most Probable Hit, MPH), a Region of Interest (ROI) is created."
                 "The data of the ROIs (namely two opposing corners) are sent to the \n"
                 "Online Selector Node (ONSEN) which also receives ROIs of the Higher Level Trigger (HLT). \n"
                 "This C++ Implementation of DATCON is the development environment, the algorithms will later \n"
                 "be transferred to FPGA, as DATCON is a hardware based online data reduction system.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParameter();

}


void
DATCONTrackingModule::initialize()
{
  storeDATCONTracks.registerInDataStore(m_storeDATCONTracksName, DataStore::c_DontWriteOut);
  m_storeDATCONTracksName = storeDATCONTracks.getName();

  storeDATCONSVDCluster.isRequired(m_storeDATCONSVDClusterName);
  storeDATCONSVDSpacePoints.isRequired(m_storeDATCONSVDSpacePointsName);

  m_storeDATCONSVDClusterName = storeDATCONSVDCluster.getName();
  m_storeDATCONSVDSpacePointsName = storeDATCONSVDSpacePoints.getName();

  if (m_useSVDSpacePoints)
    storeSVDSpacePoints.isRequired(m_storeSVDSpacePointsName);

  storeDATCONRecoTracks.registerInDataStore(m_storeDATCONRecoTracksName, DataStore::c_DontWriteOut);
  m_storeDATCONRecoTracksName = storeDATCONRecoTracks.getName();

  storeRecoHitInformation.registerInDataStore(m_storeRecoHitInformationName, DataStore::c_DontWriteOut);
  m_storeRecoHitInformationName = storeRecoHitInformation.getName();

  storeRecoHitInformation.registerRelationTo(storeDATCONSVDCluster, DataStore::c_Event, DataStore::c_DontWriteOut);
  storeDATCONSVDCluster.registerRelationTo(storeRecoHitInformation, DataStore::c_Event, DataStore::c_DontWriteOut);

  storeDATCONRecoTracks.registerRelationTo(storeRecoHitInformation);

  RecoTrack::registerRequiredRelations(storeDATCONRecoTracks,
                                       "", m_storeDATCONSVDClusterName, "",
                                       "", "", m_storeRecoHitInformationName);

  storeMCParticles.isOptional();

  if (m_independentSectors) {
    ArrayOfActiveSectorsPhiHS = new int* [m_nVertSectorsU];
    for (int i = 0; i < m_nVertSectorsU; ++i) {
      ArrayOfActiveSectorsPhiHS[i] = new int[m_nAngleSectorsU];
    }

    ArrayOfActiveSectorsThetaHS = new int* [m_nVertSectorsV];
    for (int i = 0; i < m_nVertSectorsV; ++i) {
      ArrayOfActiveSectorsThetaHS[i] = new int[m_nAngleSectorsV];
    }
  } else {
    ArrayOfActiveSectorsPhiHS = new int* [(int)pow(2, m_maxIterationsU + 1)];
    for (int i = 0; i < (int)pow(2, m_maxIterationsU + 1); ++i) {
      ArrayOfActiveSectorsPhiHS[i] = new int[(int)pow(2, m_maxIterationsU + 1)];
    }

    ArrayOfActiveSectorsThetaHS = new int* [(int)pow(2, m_maxIterationsV + 1)];
    for (int i = 0; i < (int)pow(2, m_maxIterationsV + 1); ++i) {
      ArrayOfActiveSectorsThetaHS[i] = new int[(int)pow(2, m_maxIterationsV + 1)];
    }
  }

}


void
DATCONTrackingModule::event()
{
  uHough.clear();
  uHoughCand.clear();
  uTrackCand.clear();
  vHough.clear();
  vHoughCand.clear();
  vTrackCand.clear();
  uHoughSpaceClusterCand.clear();
  vHoughSpaceClusterCand.clear();
  DATCONTracks.clear();

  if (m_useHoughSpaceClustering) {

    if (m_independentSectors) {
      for (int i = 0; i < m_nVertSectorsU; i++) {
        for (int j = 0; j < m_nAngleSectorsU; j++) {
          ArrayOfActiveSectorsPhiHS[i][j] = 0;
        }
      }
      for (int i = 0; i < m_nVertSectorsV; i++) {
        for (int j = 0; j < m_nAngleSectorsV; j++) {
          ArrayOfActiveSectorsThetaHS[i][j] = 0;
        }
      }
    } else {
      for (int i = 0; i < (int)pow(2, m_maxIterationsU + 1); i++) {
        for (int j = 0; j < (int)pow(2, m_maxIterationsU + 1); j++) {
          ArrayOfActiveSectorsPhiHS[i][j] = 0;
        }
      }
      for (int i = 0; i < (int)pow(2, m_maxIterationsV + 1); i++) {
        for (int j = 0; j < (int)pow(2, m_maxIterationsV + 1); j++) {
          ArrayOfActiveSectorsThetaHS[i][j] = 0;
        }
      }
    }

    activeSectorVectorPhi.clear();
    activeSectorVectorTheta.clear();
  }


  if (m_useDATCONSVDSpacePoints && m_useSVDSpacePoints) {
    B2WARNING("Using both DATCONSVDSpacePoints and SVDSpacePoints is not possible! Using DATCONSVDSpacePoints.");
    prepareDATCONSVDSpacePoints();
  } else if (m_useDATCONSVDSpacePoints && !m_useSVDSpacePoints) {
    prepareDATCONSVDSpacePoints();
  } else if (!m_useDATCONSVDSpacePoints && m_useSVDSpacePoints) {
    prepareSVDSpacePoints();
  } else if (!m_useDATCONSVDSpacePoints && !m_useSVDSpacePoints) {
    B2WARNING("You should use some SpacePoints at least... Using DATCONSVDSpacePoints.");
    prepareDATCONSVDSpacePoints();
  }

  if (m_usePhase2Simulation) {
    // ATTENTION TODO FIXME : This still has to be implemented!!!
    // So far no phase 2 specific algorithms have been implemented and tested!
    B2WARNING("This mode is not yet implemented, nothing will happen! Return...");
    return;
  }


  TVector2 v1_s, v2_s, v3_s, v4_s;

  /* Hough transformation */
  houghTrafo2d(uClusters, true);     // with conformal transformation in r-phi
  houghTrafo2d(vClusters, false);    // no conformal transformation in z

  /*
  * Run hough tracking on U-Side
  */
  /* Set start values */
  v1_s.Set(-M_PI,  m_rectSizeU);
  v2_s.Set(M_PI,   m_rectSizeU);
  v3_s.Set(M_PI,  -m_rectSizeU);
  v4_s.Set(-M_PI, -m_rectSizeU);
  /* Run intercept finder */
  if (m_independentSectors) {
    slowInterceptFinder2d(uHough, true);
  } else {
    fastInterceptFinder2d(uHough, true, v1_s, v2_s, v3_s, v4_s, 0, m_maxIterationsU);
  }

  /*
  * Run hough tracking on V-Side
  */
  /* Set start values */
  v1_s.Set(-M_PI,  m_rectSizeV);
  v2_s.Set(0.,     m_rectSizeV);
  v3_s.Set(0.,    -m_rectSizeV);
  v4_s.Set(-M_PI, -m_rectSizeV);
  /* Run intercept finder */
  if (m_independentSectors) {
    slowInterceptFinder2d(vHough, false);
  } else {
    fastInterceptFinder2d(vHough, false, v1_s, v2_s, v3_s, v4_s, 0, m_maxIterationsV);
  }

  if (m_useHoughSpaceClustering) {
    FindHoughSpaceCluster(true);
    FindHoughSpaceCluster(false);
  }

  /* Purify candidates (when debug level >= 2) */
  if (m_usePurifier) {
    purifyTrackCandsList();
  }

  /* Run trackCand merger */
  if (m_useTrackCandMerger) {
    trackCandMerger();
  }

  /* Find and combine compatible sets and create real tracks */
  findandcombine3d();

  /* Run trackCand merger */
  if (m_useTrackMerger) {
    trackMerger();
  }

}


void
DATCONTrackingModule::terminate()
{

  if (m_independentSectors) {
    for (int i = 0; i < m_nVertSectorsU; ++i) {
      delete [] ArrayOfActiveSectorsPhiHS[i];
    }

    for (int i = 0; i < m_nVertSectorsV; ++i) {
      delete [] ArrayOfActiveSectorsThetaHS[i];
    }
  } else {
    for (int i = 0; i < (int)pow(2, m_maxIterationsU + 1); ++i) {
      delete [] ArrayOfActiveSectorsPhiHS[i];
    }

    for (int i = 0; i < (int)pow(2, m_maxIterationsV + 1); ++i) {
      delete [] ArrayOfActiveSectorsThetaHS[i];
    }
  }

  delete [] ArrayOfActiveSectorsPhiHS;
  delete [] ArrayOfActiveSectorsThetaHS;

}
