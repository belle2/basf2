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

  storeHoughCluster.registerInDataStore(m_storeHoughClusterName, DataStore::c_DontWriteOut);
  m_storeHoughClusterName = storeHoughCluster.getName();

  storeDATCONRecoTracks.registerInDataStore(m_storeDATCONRecoTracksName, DataStore::c_DontWriteOut);
  m_storeDATCONRecoTracksName = storeDATCONRecoTracks.getName();

  storeDATCONSVDCluster.isRequired(m_storeDATCONSVDClusterName);
  storeDATCONSVDSpacePoints.isRequired(m_storeDATCONSVDSpacePointsName);

//   storeSVDSpacePoints.isOptional(m_storeSVDSpacePointsName);
  storeSVDSpacePoints.isRequired(m_storeSVDSpacePointsName);

  storeDATCONSVDSpacePoints.isRequired(m_storeDATCONSVDSpacePointsName);
  m_storeDATCONSVDSpacePointsName = storeDATCONSVDSpacePoints.getName();
  storeDATCONSVDCluster.isRequired(m_storeDATCONSVDClusterName);
  m_storeDATCONSVDClusterName = storeDATCONSVDCluster.getName();

  storeDATCONRecoTracks.registerInDataStore(m_storeDATCONRecoTracksName, DataStore::c_DontWriteOut);
  m_storeDATCONRecoTracksName = storeDATCONRecoTracks.getName();

  m_storeRecoHitInformation.registerInDataStore(m_storeRecoHitInformationName, DataStore::c_DontWriteOut);
  m_storeRecoHitInformationName = m_storeRecoHitInformation.getName();

  m_storeRecoHitInformation.registerRelationTo(storeDATCONSVDCluster, DataStore::c_Event, DataStore::c_DontWriteOut);
  storeDATCONSVDCluster.registerRelationTo(m_storeRecoHitInformation, DataStore::c_Event, DataStore::c_DontWriteOut);

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

  eventnumber = 0;
}


void
DATCONTrackingModule::event()
{

  TVector3 vec, abs_pos;
//   float u, v;
  std::map<int, TVector3>::iterator iter;

  eventnumber++;
  resetEventVariables();

  if (m_useDATCONSVDSpacePoints) {
    prepareDATCONSVDSpacePoints();
  } else if (m_useSVDSpacePoints) {
    prepareSVDSpacePoints();
  }

  if (m_usePhase2Simulation) {

    /* Save strips for FPGA conversion */
    if (m_saveStrips) {
      saveStrips();
    }
  }


  TVector2 v1_s, v2_s, v3_s, v4_s;
  double rect_size;
  vector<houghDbgPair> v_rect, u_rect;
  vector<houghDbgPair> v_rectX, v_rectY;
  bool countStrips = m_countStrips;

  ActiveSectorsPhiHS.clear();
  ActiveSectorsThetaHS.clear();

  /* Hough transformation */
  houghTrafo2d(uClusters, true,  true);     // with conformal transformation in r-phi
  houghTrafo2d(vClusters, false, false);    // no conformal transformation in z

  /*
  * Run hough tracking on U-Side
  */
  /* Initial rect size parameters for Hough trafo */
  rect_size = m_rectSizeU;
  /* Set start values */
  v1_s.Set(-M_PI,  rect_size);
  v2_s.Set(M_PI,  rect_size);
  v3_s.Set(M_PI, -rect_size);
  v4_s.Set(-M_PI, -rect_size);
  /* Run intercept finder */
  if (m_independentSectors) {
    slowInterceptFinder2d(uHough, true, u_rect, m_minimumLines);
  } else {
    fastInterceptFinder2d(uHough, true, v1_s, v2_s, v3_s, v4_s, 0, m_maxIterationsU, u_rect, m_minimumLines);
  }

  /* Debug */
  if (m_writeHoughSectors) {
    gplotRect("dbg/u_rect.plot", u_rect);
  }

  /*
  * Run hough tracking on V-Side
  */
  /* Initial rect size parameters for Hough trafo */
  rect_size = m_rectSizeV;
  /* Set start values */
  v1_s.Set(-M_PI,  rect_size);
  v2_s.Set(0.,  rect_size);
  v3_s.Set(0., -rect_size);
  v4_s.Set(-M_PI, -rect_size);
  /* Run intercept finder */
  if (m_independentSectors) {
    slowInterceptFinder2d(vHough, false, v_rect, m_minimumLines);
  } else {
    fastInterceptFinder2d(vHough, false, v1_s, v2_s, v3_s, v4_s, 0, m_maxIterationsV, v_rect, m_minimumLines);
  }

  /* Debug */
  if (m_writeHoughSectors) {
    gplotRect("dbg/v_rect.plot", v_rect);
  }


  if (m_useHoughSpaceClustering) {
    FindHoughSpaceCluster(true);
    FindHoughSpaceCluster(false);
  }

  /* Purify candidates (when debug level >= 2) */
  if (m_usePurifier) {
    m_countStrips = true;
    purifyTrackCandsList();
    m_countStrips = countStrips;
  }

  /* Run trackCand merger */
  if (m_useTrackCandMerger) {
    trackCandMerger();
  }

  /* Find and combine compatible sets and create real tracks */
  fac3d();

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
