/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdDQM/SVDDQMEfficiencyModule.h>
#include "TDirectory.h"

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMEfficiency)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMEfficiencyModule::SVDDQMEfficiencyModule() : HistoModule(), m_geoCache(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("Create basic histograms to the compute average sensor efficiency.");

  // What exactly is needed for this to be true?
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("Clusters", m_svdClustersName, "name of StoreArray with SVD cluster.", std::string(""));
  addParam("Intercepts", m_interceptsName, "name of StoreArray with SVDIntercepts.", std::string(""));

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("SVDEfficiency"));

  addParam("binsU", m_u_bins, "histogram bins in u direction.", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction.", int(6));


  addParam("saveExpertHistos", m_saveExpertHistos, "if True, save additional histograms.", bool(false));

  addParam("minSVDHits", m_minSVDHits, "Number of SVD hits required in a track to be considered.", 1u);
  addParam("minCDCHits", m_minCDCHits, "Number of CDC hits required in a track to be considered.", 20u);

  addParam("pValCut", m_pcut, "Set a cut on the track  p-value.", double(0));
  //  addParam("d0Cut", m_d0cut, "|d0| smaller than the cut", double(0.5));
  //  addParam("z0Cut", m_z0cut, "|z0| smaller than the cut", double(2));

  addParam("momCut", m_momCut, "Set a cut on the track momentum.", double(0));
  addParam("ptCut", m_ptCut, "Set a cut on the track transverse momentum.", double(1));

  addParam("fiducialU", m_fiducialU, "Fiducial Area, U direction.", float(0.5));
  addParam("fiducialV", m_fiducialV, "Fiducial Area, V direction.", float(0.5));
  addParam("maxHalfResidU", m_maxResidU, "half window for cluster search around intercept, U direction.", float(0.05));
  addParam("maxHalfResidV", m_maxResidV, "half window for cluster search around intercept, V direction.", float(0.05));

}


void SVDDQMEfficiencyModule::initialize()
{
  //calls the define histogram function
  REG_HISTOGRAM;

  //register the required arrays
  //Register as optional so validation for cases where they are not available still succeeds, but module will not do any meaningful work without them
  m_svdClusters.isOptional(m_svdClustersName);
  m_intercepts.isOptional(m_interceptsName);
  m_recoTracks.isOptional();

}


void SVDDQMEfficiencyModule::event()
{
  if (!m_svdClusters.isValid()) {
    B2INFO("SVDClusters array is missing, no SVD efficiencies");
    return;
  }
  if (!m_intercepts.isValid()) {
    B2INFO("SVDIntercepts array is missing, no SVD efficiencies");
    return;
  }

  if (!m_recoTracks.isValid()) {
    B2INFO("RecoTracks array is missing, no SVD efficiencies");
    return;
  }


  //intercepts
  for (int inter = 0 ; inter < m_intercepts.getEntries(); inter++) {

    if (!isGoodIntercept(m_intercepts[inter]))
      continue;

    B2DEBUG(10, "this intercept is related to a good track");

    VxdID::baseType theVxdID = (VxdID::baseType)m_intercepts[inter]->getSensorID();
    double coorU = m_intercepts[inter]->getCoorU();
    double coorV = m_intercepts[inter]->getCoorV();
    VXD::SensorInfoBase info = m_geoCache.getSensorInfo(theVxdID);
    int cellU = info.getUCellID(coorU);
    int cellV = info.getVCellID(coorV);

    const VXD::SensorInfoBase& theSensorInfo = m_geoCache.getSensorInfo(theVxdID);
    if (theSensorInfo.inside(coorU, coorV, -m_fiducialU, -m_fiducialV)) {

      //This track should be on the sensor
      if (m_saveExpertHistos)
        m_h_track_hits[theVxdID]->Fill(cellU, cellV);

      m_TrackHits->fill(theVxdID, 0, 1);
      m_TrackHits->fill(theVxdID, 1, 1);

      bool foundU = false;
      bool foundV = false;

      //loop on clusters
      for (int cls = 0 ; cls < m_svdClusters.getEntries(); cls++) {

        VxdID::baseType clVxdID = (VxdID::baseType)m_svdClusters[cls]->getSensorID();
        if (clVxdID != theVxdID)
          continue;

        double maxResid = m_maxResidV;
        double interCoor = coorV;
        double resid = interCoor - m_svdClusters[cls]->getPosition();
        if (m_svdClusters[cls]->isUCluster()) {
          interCoor = coorU;
          maxResid = m_maxResidU;
          resid = interCoor - m_svdClusters[cls]->getPosition(coorV);
        }


        if (resid < maxResid) {
          if (m_svdClusters[cls]->isUCluster()) {
            foundU = true;
          } else
            foundV = true;
        }

        if (foundU && foundV)
          break;
      }

      if (foundU) {
        m_MatchedHits->fill(theVxdID, 1, 1);
        if (m_saveExpertHistos) m_h_matched_clusterU[theVxdID]->Fill(cellU, cellV);
      }

      if (foundV) {
        m_MatchedHits->fill(theVxdID, 0, 1);
        if (m_saveExpertHistos)m_h_matched_clusterV[theVxdID]->Fill(cellU, cellV);
      }

    }
  }

}



void SVDDQMEfficiencyModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  m_TrackHits = new SVDSummaryPlots("TrackHits@view", "Number of Tracks intercepting the @view/@side Side");
  m_MatchedHits = new SVDSummaryPlots("MatchedHits@view", "Number of Matched Clusters on the @view/@side Side");

  if (!m_saveExpertHistos) {
    oldDir->cd();
    return;
  }

  std::vector<VxdID> sensors = m_geoCache.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_geoCache.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    //Only interested in SVD sensors

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    int nu = info.getUCells();
    int nv = info.getVCells();

    m_h_track_hits[avxdid] = new TH2D("track_hits_" + buff, "tracks through sensor " + buff,
                                      m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    m_h_matched_clusterU[avxdid] = new TH2D("matched_clusterU_" + buff, "track intersections with a matched U cluster" + buff,
                                            m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    m_h_matched_clusterV[avxdid] = new TH2D("matched_clusterV_" + buff, "track intersections with a matched V cluster" + buff,
                                            m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
  }
  // cd back to root directory
  oldDir->cd();
}



bool SVDDQMEfficiencyModule::isGoodIntercept(SVDIntercept* inter)
{

  RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(inter);

  if (theRC.size() == 0)
    return false;


  //If fit failed assume position pointed to is useless anyway
  if (!theRC[0]->wasFitSuccessful()) return false;

  if (theRC[0]->getNumberOfSVDHits() < m_minSVDHits) return false;

  if (theRC[0]->getNumberOfCDCHits() < m_minCDCHits) return false;

  const genfit::FitStatus* fitstatus = theRC[0]->getTrackFitStatus();
  if (fitstatus->getPVal() < m_pcut) return false;
  //  if (fabs(fitstatus->getD0() < m_d0cut) return false;
  //  if (fabs(fitstatus->getZ0() < m_z0cut) return false;



  genfit::MeasuredStateOnPlane trackstate;
  trackstate = theRC[0]->getMeasuredStateOnPlaneFromFirstHit();
  if (trackstate.getMom().Mag() < m_momCut) return false;

  if (trackstate.getMom().Perp() < m_ptCut) return false;

  return true;

}
