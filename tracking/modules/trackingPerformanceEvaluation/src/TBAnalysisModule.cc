
#include <tracking/modules/trackingPerformanceEvaluation/TBAnalysisModule.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <genfit/MaterialEffects.h>
#include <geometry/GeometryManager.h>
#include <TGeoManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <time.h>
#include <list>
#include <genfit/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(TBAnalysis)

TBAnalysisModule::TBAnalysisModule() : Module()
{

  setDescription("This module performs the on-site offline analysis of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("TBAnalysis_output.root"));
}

TBAnalysisModule::~TBAnalysisModule()
{

}

void TBAnalysisModule::initialize()
{
  m_goodL1inter = 0;
  m_goodL2inter = 0;

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  m_h1_pValue = createHistogram1D("h1pValue", "pValue of the fit", 1000, 0, 1, "pValue", m_histoList);

  m_h1_uResid_L1 = createHistogram1D("h1uResidL1", "L1 u residuals (intercept - pixel)", 200, -2, 2, "u resid", m_histoList);
  m_h1_vResid_L1 = createHistogram1D("h1vResidL1", "L1 v residuals (intercept - pixel)", 100, -5, 5, "v resid", m_histoList);
  m_h1_uResid_L2 = createHistogram1D("h1uResidL2", "L2 u residuals (intercept - pixel)", 200, -2, 2, "u resid", m_histoList);
  m_h1_vResid_L2 = createHistogram1D("h1vResidL2", "L2 v residuals (intercept - pixel)", 100, -5, 5, "v resid", m_histoList);

  m_h2_hitMap_vL1_uL1 = createHistogram2D("L1hitMap", "L1 HitMap", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)", m_histoList);
  m_h2_hitMap_vL2_uL2 = createHistogram2D("L2hitMap", "L2 HitMap", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)", m_histoList);

  m_h2_interMap_vL1_uL1 = createHistogram2D("L1interMap", "L1 Intercepts Map", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)",
                                            m_histoList);
  m_h2_interMap_vL2_uL2 = createHistogram2D("L2interMap", "L2 Intercepts Map", 300, -2.5, 3.5, "v (cm)", 100, -0.8, 0.8, "u (cm)",
                                            m_histoList);


  m_h2_corr_uL3_uL1 = createHistogram2D("h2_corr_uL3_uL1", "correlations: u L3 vs u L1", 1000, -2, 2, "SVD L3 u strip", 1000, -2, 2,
                                        "PXD L1 u coord", m_histoList);
  m_h2_corr_uL3_uL2 = createHistogram2D("h2_corr_uL3_uL2", "correlations: u L3 vs u L2", 1000, -2, 2, "SVD L3 u strip", 1000, -2, 2,
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
                                        "SVD L4 u coord", m_histoList);

}

void TBAnalysisModule::beginRun()
{

}

void TBAnalysisModule::event()
{

  // SVD objects
  //  StoreArray<SVDDigit> SVDDigits;
  StoreArray<SVDCluster> svdClusters;

  //PXD objects
  StoreArray<PXDCluster> pxdClusters;

  //track-fit objects
  StoreArray<genfit::Track> tracks("offline_gfTracks");

  //ROIs objects
  //  StoreArray<ROIid> ROIList;
  //  StoreArray<PXDIntercept> PXDIntercepts("offline_PXDIntercepts");

  B2DEBUG(1, "%%%%%%%% NEW EVENT,  number of tracks =  " << tracks.getEntries());

  if (tracks.getEntries() == 0)
    return;

  bool isInFiducialL1 = false;
  bool isInFiducialL2 = false;

  BOOST_FOREACH(genfit::Track & track, tracks) {


    const TrackFitResult* fitResult = DataStore::getRelatedFromObj<TrackFitResult>(&track);
    if (fitResult == NULL)
      continue;

    m_h1_pValue->Fill(fitResult->getPValue());

    //    if(fitResult->getPValue() < 0.0001)
    //      continue;

    RelationVector<PXDIntercept> PXDIntercept_fromTrack = DataStore::getRelationsFromObj<PXDIntercept>(&track, "offline_PXDIntercepts");
    if (PXDIntercept_fromTrack.size() == 0)
      continue;

    float L1uInter = -10;
    float L2uInter = -10;
    float L1vInter = -10;
    float L2vInter = -10;

    isInFiducialL1 = false;
    isInFiducialL2 = false;

    PXDIntercept* theInterceptL1 = PXDIntercept_fromTrack[0];
    //    if( VxdID(theIntercept->getSensorID()).getLayerNumber() == 1){
    L1uInter = theInterceptL1->getCoorU();
    L1vInter = theInterceptL1->getCoorV();
    //    }   else {
    //      cout<<"layer 2!"<<endl;
    PXDIntercept* theInterceptL2 = PXDIntercept_fromTrack[1];
    L2uInter = theInterceptL2->getCoorU();
    L2vInter = theInterceptL2->getCoorV();
    //    }
    m_h2_interMap_vL1_uL1->Fill(L1vInter, L1uInter);
    m_h2_interMap_vL2_uL2->Fill(L2vInter, L2uInter);
//    if (fitResult->getPValue() < 0.0001 )
    //      continue;

    if (abs(L1uInter) < 5 && abs(L1vInter) < 20)
      isInFiducialL1 = true;

    if (abs(L2uInter) < 5 && abs(L2vInter) < 28)
      isInFiducialL2 = true;

    RelationVector<genfit::TrackCand> TrackCands_fromTrack = DataStore::getRelationsToObj<genfit::TrackCand>(&track,
                                                             "offline_caTracks");

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

    bool aClusterOnL1 = false;
    bool aClusterOnL2 = false;

    for (int pxd = 0; pxd < pxdClusters.getEntries(); pxd++) {
      if (pxdClusters[pxd]->getSensorID().getLayerNumber() == 1) {
        L1u = pxdClusters[pxd]->getU();
        L1v = pxdClusters[pxd]->getV();
        m_h2_corr_uL3_uL1->Fill(L3u, L1u);
        m_h2_corr_uL4_uL1->Fill(L4u, L1u);
        m_h2_corr_vL3_uL1->Fill(L3v, L1u);
        m_h2_corr_vL3_vL1->Fill(L3v, L1v);
        m_h2_hitMap_vL1_uL1->Fill(L1v, L1u);
        if ((L1v - L1vInter) * (L1v - L1vInter) + (L1u - L1uInter) * (L1u - L1uInter) < 0.4 * 04)
          aClusterOnL1 = true;
      } else {
        L2u = pxdClusters[pxd]->getU();
        L2v = pxdClusters[pxd]->getV();
        m_h2_corr_uL3_uL2->Fill(L3u, L2u);
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
          m_h1_uResid_L2->Fill(L2uInter - L2u);
          m_h1_vResid_L2->Fill(L2vInter - L2v);
        }


      if (aClusterOnL2 && isInFiducialL1)
        if (pxdClusters[pxd]->getSensorID().getLayerNumber() == 1) {
          L1u = pxdClusters[pxd]->getU();
          L1v = pxdClusters[pxd]->getV();
          m_h1_uResid_L1->Fill(L1uInter - L1u);
          m_h1_vResid_L1->Fill(L1vInter - L1v);
        }

    }
    m_h2_corr_uL3_uL4->Fill(L3u, L4u);

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
