#include <tracking/modules/trackingPerformanceEvaluation/TBAnalysisModule.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <genfit/MaterialEffects.h>
#include <geometry/GeometryManager.h>
#include <TGeoManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <time.h>
#include <list>
#include <genfit/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(TBAnalysis)

TBAnalysisModule::TBAnalysisModule() : Module()
{

  setDescription("This module performs the on-site offline analysis of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("TBAnalysis_output.root"));

  addParam("TriggerDivider", m_TrigDiv, "TrigDiv Divider.", 1);

  addParam("TrackCandListName", m_TrackCandName, "Name of TrackCands.", std::string(""));
  addParam("TrackListName", m_TrackName, "Name of Tracks.", std::string(""));
  addParam("TrackFitResultListName", m_TrackFitResultName, "Name of TrackFitResults.", std::string("offline_TrackFitResults"));
  addParam("PXDInterceptListName", m_PXDInterceptName, "Name of PXDIntercepts.", std::string(""));
  addParam("ROIidListName", m_ROIidName, "Name of ROIs.", std::string(""));
}

TBAnalysisModule::~TBAnalysisModule()
{

}

void TBAnalysisModule::initialize()
{
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.isRequired();

  StoreArray<genfit::TrackCand> trackCands;
  trackCands.isRequired(m_TrackCandName);

  StoreArray<genfit::Track> tracks;
  tracks.isRequired(m_TrackName);

  StoreArray<TrackFitResult> trackFitResults;
  trackFitResults.isRequired(m_TrackFitResultName);

  StoreArray<PXDIntercept> pxdIntercepts;
  pxdIntercepts.isRequired(m_PXDInterceptName);

  StoreArray<ROIid> roiIDs;
  roiIDs.isRequired(m_ROIidName);

  B2INFO("      TrackCands: " << m_TrackCandName);
  B2INFO("          Tracks: " << m_TrackName);
  B2INFO(" TrackFitResults: " << m_TrackFitResultName);
  B2INFO("   PXDIntercepts: " << m_PXDInterceptName);
  B2INFO("            ROIs: " << m_ROIidName);

  m_goodL1inter = 0;
  m_goodL2inter = 0;

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  m_h1_pValue = createHistogram1D("h1pValue", "pValue of the fit", 1000, 0, 1, "pValue", m_histoList);
  m_h1_mom = createHistogram1D("h1mom", "fitted track momentum", 100, 0, 7, "momentum", m_histoList);


  m_h1_nROIs_odd = createHistogram1D("h1nROIs_trgDivNot0", "number of ROIs per event", 10, 0, 10, "n ROIs", m_histoList);
  m_h1_nROIs_even = createHistogram1D("h1nROIs_trgDiv0", "number of ROIs per event", 10, 0, 10, "n ROIs", m_histoList);

  m_h1_uwidthROI_odd = createHistogram1D("h1uWidth_trgDivNot0", "L1+L2 u ROI width", 250, 0, 250, "u width", m_histoList);
  m_h1_vwidthROI_odd = createHistogram1D("h1vWidth_trgDivNot0", "L1+L2 v ROI width", 768, 0, 768, "v width", m_histoList);
  m_h1_uwidthROI_even = createHistogram1D("h1uWidth_trgDiv0", "L1+L2 u ROI width", 250, 0, 250, "u width", m_histoList);
  m_h1_vwidthROI_even = createHistogram1D("h1vWidth_trgDiv0", "L1+L2 v ROI width", 768, 0, 768, "v width", m_histoList);


  m_h1_uResid_L1_even = createHistogram1D("h1uResidL1_trgDiv0", "L1 u residuals (intercept - pixel)", 1000, -2, 2, "u resid",
                                          m_histoList);
  m_h1_vResid_L1_even = createHistogram1D("h1vResidL1_trgDiv0", "L1 v residuals (intercept - pixel)", 1000, -5, 5, "v resid",
                                          m_histoList);
  m_h1_uResid_L2_even = createHistogram1D("h1uResidL2_trgDiv0", "L2 u residuals (intercept - pixel)", 1000, -2, 2, "u resid",
                                          m_histoList);
  m_h1_vResid_L2_future_even = createHistogram1D("h1vResidL2_future_trgDiv0", "L2 v residuals (intercept - pixel)", 1000, -5, 5,
                                                 "v resid", m_histoList);
  m_h1_uResid_L2_future_even = createHistogram1D("h1uResidL2_future_trgDiv0", "L2 u residuals (intercept - pixel)", 1000, -2, 2,
                                                 "u resid", m_histoList);
  m_h1_vResid_L2_even = createHistogram1D("h1vResidL2_trgDiv0", "L2 v residuals (intercept - pixel)", 1000, -5, 5, "v resid",
                                          m_histoList);

  m_h1_uResid_L1_odd = createHistogram1D("h1uResidL1_trgDivNot0", "L1 u residuals (intercept - pixel)", 1000, -2, 2, "u resid",
                                         m_histoList);
  m_h1_vResid_L1_odd = createHistogram1D("h1vResidL1_trgDivNot0", "L1 v residuals (intercept - pixel)", 1000, -5, 5, "v resid",
                                         m_histoList);
  m_h1_uResid_L2_odd = createHistogram1D("h1uResidL2_trgDivNot0", "L2 u residuals (intercept - pixel)", 1000, -2, 2, "u resid",
                                         m_histoList);
  m_h1_vResid_L2_odd = createHistogram1D("h1vResidL2_trgDivNot0", "L2 v residuals (intercept - pixel)", 1000, -5, 5, "v resid",
                                         m_histoList);

  m_h2_uvResidCluster_L1 = createHistogram2D("L1uvResidCluster", "u v resid L1", 3000, -2.5, 3.5, "v (cm)", 1000, -0.8, 0.8, "u (cm)",
                                             m_histoList);
  m_h2_uvResidCluster_L2 = createHistogram2D("L2uvResidCluster", "u v resid L2", 3000, -2.5, 3.5, "v (cm)", 1000, -0.8, 0.8, "u (cm)",
                                             m_histoList);
  m_h2_uvResidDigit_L1 = createHistogram2D("L1uvResidDigit", "u v resid L1", 3000, -2.5, 3.5, "v (cm)", 1000, -0.8, 0.8, "u (cm)",
                                           m_histoList);
  m_h2_uvResidDigit_L2 = createHistogram2D("L2uvResidDigit", "u v resid L2", 3000, -2.5, 3.5, "v (cm)", 1000, -0.8, 0.8, "u (cm)",
                                           m_histoList);


  //  m_h2_PXDCluster_L1 = createHistogram2D("L1pxdCLuster", "PXDClusters L1", 2, -2, 2, "v (cm)", 2, 0, 0.35, "u (cm)", m_histoList);
  m_h2_Intercepts_L1 = createHistogram2D("L1intercepts", "Intercepts in Fiducial Area of L1",  300, -2.5, 3.5, "v (cm)", 100, -0.8,
                                         0.8, "u (cm)", m_histoList);
  //  m_h2_PXDCluster_L2 = createHistogram2D("L2pxdCLuster", "PXDClusters L2", 2, -2, 2, "v (cm)", 2, 0, 0.35, "u (cm)", m_histoList);
  m_h2_Intercepts_L2 = createHistogram2D("L2intercepts", "Intercepts in Fiducial Area of L2",   300, -2.5, 3.5, "v (cm)", 100, -0.8,
                                         0.8, "u (cm)", m_histoList);


  m_h2_hitMap_vL1_uL1 = createHistogram2D("L1hitMap", "L1 HitMap", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)", m_histoList);
  m_h2_hitMap_vL2_uL2 = createHistogram2D("L2hitMap", "L2 HitMap", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)", m_histoList);

  m_h2_interMap_vL1_uL1 = createHistogram2D("L1interMap", "L1 Intercepts Map", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)",
                                            m_histoList);
  m_h2_interMap_vL2_uL2 = createHistogram2D("L2interMap", "L2 Intercepts Map", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)",
                                            m_histoList);


  m_h2_corr_uL3_uL1_even = createHistogram2D("h2_corr_uL3_uL1_trgDiv0", "correlations: u L3 vs u L1", 1000, -2, 2, "SVD L3 u strip",
                                             1000, -2, 2,
                                             "PXD L1 u coord", m_histoList);
  m_h2_corr_uL3_uL1_odd = createHistogram2D("h2_corr_uL3_uL1_trgDivNot0", "correlations: u L3 vs u L1", 1000, -2, 2, "SVD L3 u strip",
                                            1000, -2, 2,
                                            "PXD L1 u coord", m_histoList);
  m_h2_corr_uL3_uL2_even = createHistogram2D("h2_corr_uL3_uL2_trgDiv0", "correlations: u L3 vs u L2", 1000, -2, 2, "SVD L3 u strip",
                                             1000, -2, 2,
                                             "PXD L2 u coord", m_histoList);
  m_h2_corr_uL3_uL2_odd = createHistogram2D("h2_corr_uL3_uL2_trgDivNot0", "correlations: u L3 vs u L2", 1000, -2, 2, "SVD L3 u strip",
                                            1000, -2, 2,
                                            "PXD L2 u coord", m_histoList);
  m_h2_corr_vL3_vL1 = createHistogram2D("h2_corr_vL3_vL1", "correlations: v L3 vs v L1", 1000, -2, 2, "SVD L3 v strip", 1000, -2, 2,
                                        "PXD L1 v coord", m_histoList);
  m_h2_corr_vL3_vL2 = createHistogram2D("h2_corr_vL3_vL2", "correlations: v L3 vs v L2", 1000, -2, 2, "SVD L3 v strip", 1000, -2, 2,
                                        "PXD L2 v coord", m_histoList);

  m_h2_corr_uL4_uL1 = createHistogram2D("h2_corr_uL4_uL1", "correlations: u L4 vs u L1", 1000, -2, 2, "SVD L4 u strip", 1000, -2, 2,
                                        "PXD L1 u coord", m_histoList);

  m_h2_corr_vL3_uL1 = createHistogram2D("h2_corr_vL3_uL1", "correlations: v L3 vs u L1", 1000, -2, 2, "SVD L3 v strip", 1000, -2, 2,
                                        "PXD L1 u coord", m_histoList);
  m_h2_corr_uL3_uL4 = createHistogram2D("h2_corr_uL3_uL4", "correlations: u L3 vs u L4", 1000, -2, 2, "SVD L3 u strip", 1000, -2, 2,
                                        "SVD L4 u strip", m_histoList);
  m_h2_corr_vL3_vL4 = createHistogram2D("h2_corr_vL3_vL4", "correlations: v L3 vs v L4", 1000, -2, 2, "SVD L3 v strip", 1000, -2, 2,
                                        "SVD L4 v strip", m_histoList);

  //  m_h2_corr_uL1_uL2 = createHistogram2D("h2_corr_uL1_uL2", "correlations: u L1 vs u L2", 1000, -2, 2, "PXD L1 u", 1000, -2, 2,
  //                                        "PXD L2 u", m_histoList);
  //  m_h2_corr_vL1_vL2 = createHistogram2D("h2_corr_vL1_vL2", "correlations: v L1 vs v L2", 1000, -2, 2, "PXD L1 v", 1000, -2, 2,
  //                                        "PXD L2 v", m_histoList);

}

void TBAnalysisModule::beginRun()
{

}

void TBAnalysisModule::event()
{

  static vector<PXDIntercept> interceptL1_twoEvtsAgo;
  static vector<PXDIntercept> interceptL2_twoEvtsAgo;

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  unsigned int evtNr = eventMetaDataPtr->getEvent();

  // SVD objects
  //  StoreArray<SVDDigit> SVDDigits;
  StoreArray<SVDCluster> svdClusters;

  //PXD objects
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<PXDDigit> pxdDigits;

  //track-fit objects
  StoreArray<genfit::Track> tracks(m_TrackName);
  StoreArray<genfit::Track> offlinetracks("offlinegfTracks");

  //ROIs objects
  StoreArray<ROIid> ROIids(m_ROIidName);
  StoreArray<PXDIntercept> PXDIntercepts(m_PXDInterceptName);
  if (evtNr % m_TrigDiv == 0)
    m_h1_nROIs_even->Fill(ROIids.getEntries());
  else
    m_h1_nROIs_odd->Fill(ROIids.getEntries());


  B2DEBUG(1, "%%%%%%%% NEW EVENT,  number of tracks =  " << tracks.getEntries());

  if (tracks.getEntries() == 0)
    return;

  bool isInFiducialL1 = false;
  bool isInFiducialL2 = false;

  if (offlinetracks.getEntries() == 0)
    return;

  const TrackFitResult* aFitResult = DataStore::getRelated<TrackFitResult>(offlinetracks[0], "offlineTrackFitResults");
  if (aFitResult == NULL)
    return;

  m_h1_mom->Fill(aFitResult->getMomentum().Mag());
  if (abs(aFitResult->getMomentum().Mag() - 3.5) > 1.5)
    return;


  BOOST_FOREACH(genfit::Track & track, tracks) {


    const TrackFitResult* fitResult = DataStore::getRelated<TrackFitResult>(&track, m_TrackFitResultName);

    if (fitResult != NULL) {
      m_h1_pValue->Fill(fitResult->getPValue());
    } else
      continue;

    if (fitResult->getPValue() < 0.01)
      continue;

    RelationVector<PXDIntercept> PXDIntercept_fromTrack = DataStore::getRelationsWithObj<PXDIntercept>(&track, m_PXDInterceptName);

    if (PXDIntercept_fromTrack.size() == 0)
      continue;

    float L1uInter = -10;
    float L2uInter = -10;
    float L1vInter = -10;
    float L2vInter = -10;

    isInFiducialL1 = false;
    isInFiducialL2 = false;

    if (PXDIntercept_fromTrack.size() > 0) {
      PXDIntercept* theIntercept0 = PXDIntercept_fromTrack[0];
      if (VxdID(theIntercept0->getSensorID()).getLayerNumber() == 1) {
        L1uInter = theIntercept0->getCoorU();
        L1vInter = theIntercept0->getCoorV();
      }   else {
        L2uInter = theIntercept0->getCoorU();
        L2vInter = theIntercept0->getCoorV();
      }
    }
    if (PXDIntercept_fromTrack.size() > 1) {
      PXDIntercept* theIntercept1 = PXDIntercept_fromTrack[1];
      if (VxdID(theIntercept1->getSensorID()).getLayerNumber() == 1) {
        L1uInter = theIntercept1->getCoorU();
        L1vInter = theIntercept1->getCoorV();
      }   else {
        L2uInter = theIntercept1->getCoorU();
        L2vInter = theIntercept1->getCoorV();
      }
    }

    if (L1vInter != -10)
      m_h2_interMap_vL1_uL1->Fill(L1vInter, L1uInter);
    if (L2vInter != -10)
      m_h2_interMap_vL2_uL2->Fill(L2vInter, L2uInter);


    if ((L1uInter > 0.05 && L1uInter < 0.25) && abs(L1vInter) < 2)
      //4 links active
      //    if ((abs(L1uInter)<0.5) && abs(L1vInter) < 2)
      isInFiducialL1 = true;

    if (isInFiducialL1)
      m_h2_Intercepts_L1->Fill(L1vInter, L1uInter);

    if (abs(L2uInter) < 0.5 && abs(L2vInter) < 2)
      isInFiducialL2 = true;

    if (isInFiducialL2)
      m_h2_Intercepts_L2->Fill(L2vInter, L2uInter);


    RelationVector<genfit::TrackCand> TrackCands_fromTrack = DataStore::getRelationsWithObj<genfit::TrackCand>(&track,
                                                             m_TrackCandName);

    genfit::TrackCandHit* thehitTC = 0;


    if (TrackCands_fromTrack.size() == 0)
      continue;

    float L1u = -10;
    float L1v = -10;
    float L2u = -10;
    float L2v = -10;
    float L3u = -10;
    float L3v = -10;
    float L4u = -10;
    float L4v = -10;

    for (int hitTC = 0; hitTC < (int)TrackCands_fromTrack[0]->getNHits(); hitTC++) {

      thehitTC = TrackCands_fromTrack[0]->getHit(hitTC);

      int hitId = thehitTC->getHitId();
      int detId = thehitTC->getDetId();

      if (detId == Const::SVD)
        if (hitId < svdClusters.getEntries()) {
          if (svdClusters[hitId]->getSensorID().getLayerNumber() == 3) {
            if (svdClusters[hitId]->isUCluster())
              L3u = svdClusters[hitId]->getPosition();
            else
              L3v = svdClusters[hitId]->getPosition();
          }
          if (svdClusters[hitId]->getSensorID().getLayerNumber() == 4) {
            if (svdClusters[hitId]->isUCluster())
              L4u = svdClusters[hitId]->getPosition();
            else
              L4v = svdClusters[hitId]->getPosition();
          }
        }
    }

    bool aClusterOnL1 = false;
    bool aClusterOnL2 = false;

    for (int pxd = 0; pxd < pxdClusters.getEntries(); pxd++) {
      if (pxdClusters[pxd]->getSensorID().getLayerNumber() == 1) {
        L1u = pxdClusters[pxd]->getU();
        L1v = pxdClusters[pxd]->getV();
        if (evtNr % m_TrigDiv == 0)
          m_h2_corr_uL3_uL1_even->Fill(L3u, L1u);
        else
          m_h2_corr_uL3_uL1_odd->Fill(L3u, L1u);
        m_h2_corr_uL4_uL1->Fill(L4u, L1u);
        m_h2_corr_vL3_uL1->Fill(L3v, L1u);
        m_h2_corr_vL3_vL1->Fill(L3v, L1v);
        m_h2_hitMap_vL1_uL1->Fill(L1v, L1u);
        if ((L1v - L1vInter) * (L1v - L1vInter) + (L1u - L1uInter) * (L1u - L1uInter) < 0.4 * 04)
          aClusterOnL1 = true;
      } else {
        L2u = pxdClusters[pxd]->getU();
        L2v = pxdClusters[pxd]->getV();
        if (evtNr % m_TrigDiv == 0)
          m_h2_corr_uL3_uL2_even->Fill(L3u, L2u);
        else
          m_h2_corr_uL3_uL2_odd->Fill(L3u, L2u);
        m_h2_corr_vL3_vL2->Fill(L3v, L2v);
        m_h2_hitMap_vL2_uL2->Fill(L2v, L2u);
        if ((L2v - L2vInter) * (L2v - L2vInter) + (L2u - L2uInter) * (L2u - L2uInter) < 0.4 * 04)
          aClusterOnL2 = true;
      }
    }

    if (aClusterOnL1 && isInFiducialL2)
      m_goodL2inter++;
    if (aClusterOnL2 && isInFiducialL1)
      m_goodL1inter++;

    for (int pxd = 0; pxd < pxdClusters.getEntries(); pxd++) {

      if (aClusterOnL1 && isInFiducialL2)
        if (pxdClusters[pxd]->getSensorID().getLayerNumber() == 2) {
          L2u = pxdClusters[pxd]->getU();
          L2v = pxdClusters[pxd]->getV();
          if (evtNr % m_TrigDiv == 0) {
            m_h1_uResid_L2_even->Fill(L2uInter - L2u);
            m_h1_vResid_L2_even->Fill(L2vInter - L2v);
          } else {
            m_h1_uResid_L2_odd->Fill(L2uInter - L2u);
            m_h1_vResid_L2_odd->Fill(L2vInter - L2v);
          }
          m_h2_uvResidCluster_L2->Fill(L2vInter - L2v, L2uInter - L2u);
        }


      if (aClusterOnL2 && isInFiducialL1)
        if (pxdClusters[pxd]->getSensorID().getLayerNumber() == 1) {
          L1u = pxdClusters[pxd]->getU();
          L1v = pxdClusters[pxd]->getV();
          if (evtNr % m_TrigDiv == 0) {
            m_h1_uResid_L1_even->Fill(L1uInter - L1u);
            m_h1_vResid_L1_even->Fill(L1vInter - L1v);
          } else {
            m_h1_uResid_L1_odd->Fill(L1uInter - L1u);
            m_h1_vResid_L1_odd->Fill(L1vInter - L1v);
          }
          m_h2_uvResidCluster_L1->Fill(L1vInter - L1v, L1uInter - L1u);
        }


    }

    m_h2_corr_uL3_uL4->Fill(L3u, L4u);
    m_h2_corr_vL3_vL4->Fill(L3v, L4v);

    bool isInL1 = false;
    bool isInL2 = false;

    for (int pxd = 0; pxd < pxdDigits.getEntries(); pxd++) {

      const PXD::SensorInfo& aSensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(pxdDigits[pxd]->getSensorID()));

      if (pxdDigits[pxd]->getSensorID().getLayerNumber() == 2) {

        float L2vCell = aSensorInfo.getVCellPosition(pxdDigits[pxd]->getVCellID());
        float L2uCell = aSensorInfo.getUCellPosition(pxdDigits[pxd]->getUCellID());

        if (aClusterOnL1 && isInFiducialL2)
          if (! isInL2)
            if (abs(L2vCell - L2vInter - 0.005) < 0.06 && abs(L2uCell - L2uInter - 0.01)  < 0.08) {
              //      if ( abs(L2v - L2vInter - 0.005) < 1 && abs(L2u - L2uInter - 0.01)  < 1){
              isInL2 = true;
              m_h2_uvResidDigit_L2->Fill(L2vInter - L2vCell - 0.005, L2uInter - L2uCell - 0.01);
            }
      } else {

        float L1vCell = aSensorInfo.getVCellPosition(pxdDigits[pxd]->getVCellID());
        float L1uCell = aSensorInfo.getUCellPosition(pxdDigits[pxd]->getUCellID());

        if (aClusterOnL2 && isInFiducialL1)
          if (! isInL1)
            if (abs(L1vCell - L1vInter - 0.09) < 0.06 && abs(L1uCell - L1uInter + 0.026)  < 0.04) {
              isInL1 = true;
              m_h2_uvResidDigit_L1->Fill(L1vInter - L1vCell - 0.09, L1uInter - L1uCell + 0.026);
            }
      }

    } //pxd digits

  } //tracks


  for (int i = 0; i < ROIids.getEntries(); i++)
    if (evtNr % m_TrigDiv == 0) {
      m_h1_vwidthROI_even->Fill(ROIids[i]->getMaxVid() - ROIids[i]->getMinVid());
      m_h1_uwidthROI_even->Fill(ROIids[i]->getMaxUid() - ROIids[i]->getMinUid());
    } else {
      m_h1_vwidthROI_odd->Fill(ROIids[i]->getMaxVid() - ROIids[i]->getMinVid());
      m_h1_uwidthROI_odd->Fill(ROIids[i]->getMaxUid() - ROIids[i]->getMinUid());
    }

  if (evtNr % m_TrigDiv == 0) {

    if ((interceptL1_twoEvtsAgo.size() > 0) && (interceptL2_twoEvtsAgo.size() > 0)) {


      for (int pxd = 0; pxd < pxdClusters.getEntries(); pxd++) {
        if (pxdClusters[pxd]->getSensorID().getLayerNumber() == 2) {
          m_h1_uResid_L2_future_even->Fill(interceptL2_twoEvtsAgo[0].getCoorU() -  pxdClusters[pxd]->getU());
          m_h1_vResid_L2_future_even->Fill(interceptL2_twoEvtsAgo[0].getCoorV() -  pxdClusters[pxd]->getV());
        }

      }
    }
    interceptL1_twoEvtsAgo.clear();
    interceptL2_twoEvtsAgo.clear();
    for (auto intercept : PXDIntercepts)
      if (VxdID(intercept.getSensorID()).getLayerNumber() == 1)
        interceptL1_twoEvtsAgo.push_back(intercept);
      else
        interceptL2_twoEvtsAgo.push_back(intercept);

  }

}


void TBAnalysisModule::endRun()
{

}


void TBAnalysisModule::terminate()
{

  B2INFO(" number of intercepts in the fiducial L1 area = " << m_goodL1inter);
  B2INFO(" number of intercepts in the fiducial L2 area = " << m_goodL2inter);

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TIter nextH(m_histoList);
    TObject* obj;
    while ((obj = nextH()))
      obj->Write();


    m_rootFilePtr->Close();
  }


}
