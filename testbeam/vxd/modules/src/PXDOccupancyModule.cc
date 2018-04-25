/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/spirit/home/support/detail/endian.hpp>

#include <testbeam/vxd/modules/PXDOccupancyModule.h>

#include "TMatrixDSym.h"
#include "TVectorD.h"

#include <iostream>

//only used for debugging!
#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace std;
using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDOccupancy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDOccupancyModule::PXDOccupancyModule() : HistoModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("writes data to a root file which can be used for the estimation of the PXD occupancy");

  // Parameter definitions
  addParam("pxdclustersname", m_pxdclustersname, "name of StoreArray with PXD cluster", std::string(""));
  addParam("pxddigitsname", m_pxddigitsname, "name of StoreArray with PXD digits", std::string(""));
  addParam("eventmetadataname", m_eventmetadataname, "name of StoreObjPtr to event meta data", std::string(""));
  addParam("tracksname", m_tracksname, "name of StoreArray with genfit::Tracks", std::string(""));

  addParam("distCut", m_distcut, "max distance in [cm] for cluster/digit to be counted to a track", double(0.0500));

  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", bool(false));
  addParam("writeTree", m_writeTree, "if true a tree with usefull info will be filled", bool(false));

}


PXDOccupancyModule::~PXDOccupancyModule()
{
}

void PXDOccupancyModule::initialize()
{
  //calls the define histogram function
  REG_HISTOGRAM;

  //register the required arrays
  m_pxdclusters.isRequired(m_pxdclustersname);
  m_pxddigits.isRequired(m_pxddigitsname);
  storeEventMetaData.isRequired(m_eventmetadataname);
}


void PXDOccupancyModule::event()
{



  StoreArray<genfit::Track> tracks(m_tracksname);
  m_nsvdtracks = tracks.getEntries();
  //std::cout << "n track " << m_tracks.getEntries() << std::endl;
  //std::cout << m_tracksname << std::endl;

  m_event = int(storeEventMetaData->getEvent());
  m_run = int(storeEventMetaData->getRun());
  m_subrun = int(storeEventMetaData->getSubrun());


  // We have to find the trigger numbers from the pxd data ...
  unsigned int pxdTriggerNr = 0x10000, triggerNr = 0x10000;
  // pxdTriggerNr is the DHC one
  // triggerNR the HLT/Onsen one
  for (auto& it : m_storeRaw) {
    if (getTrigNr(it, pxdTriggerNr, triggerNr)) break; // only first (valid) one
  }
  m_pxdTriggerNr = pxdTriggerNr;
  m_triggerNr = triggerNr;



  //loop the pxd clusters
  for (PXDCluster& acluster : m_pxdclusters) {

    VxdID aVxdID = acluster.getSensorID();
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;


    m_layernumber = aVxdID.getLayerNumber();
    m_laddernumber = aVxdID.getLadderNumber();
    m_sensornumber = aVxdID.getSensorNumber();
    m_badcluster = 1;

    // Read cluster informatiom
    m_u_clus = acluster.getU();
    m_v_clus = acluster.getV();

    m_clus_charge = acluster.getCharge();
    m_clus_seedcharge = acluster.getSeedCharge();

    m_clus_size = acluster.getSize();
    m_clus_usize = acluster.getUSize();
    m_clus_vsize = acluster.getVSize();

    TVector3 cluster_buff(m_u_clus, m_v_clus, 0);

    // find closest  track
    int besttrack = findClosestTrack(aVxdID, cluster_buff);

    if (besttrack >= 0) {

      //not sure at which position that is but the correct momentum is not really needed
      genfit::MeasuredStateOnPlane trackstate;
      genfit::FitStatus* fitstatus = NULL;
      try {
        fitstatus = tracks[besttrack]->getFitStatus(); //(const AbsTrackRep* rep = NULL)
        trackstate = tracks[besttrack]->getFittedState(false); //bool for biased or not
      } catch (...) {
        return;
      }

      bool isgood = false;
      double sig_u(-1);
      double sig_v(-1);
      TVector3 intersec_best = getTrackInterSec(info, tracks[besttrack], isgood, sig_u, sig_v);

      m_track_matched = 1; //isgood;
      m_u_fit = intersec_best.X();
      m_v_fit = intersec_best.Y();
      m_sigma_u_fit = sig_u;
      m_sigma_v_fit = sig_v;
      m_ucell_fit = info.getUCellID(m_u_fit);
      m_vcell_fit = info.getVCellID(m_v_fit);

      //track quality indicators
      m_charge_pdg = fitstatus->getCharge(); // not trackstate!
      m_fit_pValue = fitstatus->getPVal();
      m_fit_ndf = fitstatus->getNdf();
      TVector3 mom = trackstate.getMom();
      m_fit_mom = mom.Mag();
      m_fit_theta = mom.Theta();
      m_fit_phi = mom.Phi();

      const TVectorD& state  = trackstate.getState(); // 5D with elements q/p,u',v',u,v in plane system
      m_charge = (state[0] > 0) - (state[0] < 0);


    }  else {

      m_track_matched = 0;
      m_u_fit = 99;
      m_v_fit = 99;
      m_sigma_u_fit = -1;
      m_sigma_v_fit = -1;
      m_ucell_fit = -1;
      m_vcell_fit = -1;

      //track quality indicators

      m_charge_pdg = -1;
      m_charge = -99;
      m_fit_pValue = -1;
      m_fit_ndf = -1;
      m_fit_mom = -1;
      m_fit_theta = -9999;
      m_fit_phi = -9999;

    }

    if (m_writeTree) m_tree->Fill();

  }

  if (m_writeTree) m_eventtree->Fill();
}


TVector3 PXDOccupancyModule::getTrackInterSec(VXD::SensorInfoBase& SensorInfo, const genfit::Track* aTrack, bool& isgood,
                                              double& du, double& dv)
{
  //will be set true if the intersect was found
  isgood = false;

  TVector3 intersec(99999999, 9999999, 0); //point outside the sensor

  genfit::MeasuredStateOnPlane gfTrackState = aTrack->getFittedState();

  //adopted (aka stolen) from tracking/modules/pxdClusterRescue/PXDClusterRescueROIModule
  try {
    // get sensor plane
    TVector3 zeroVec(0, 0, 0);
    TVector3 uVec(1, 0, 0);
    TVector3 vVec(0, 1, 0);

    genfit::DetPlane* sensorPlane = new genfit::DetPlane();
    sensorPlane->setO(SensorInfo.pointToGlobal(zeroVec, m_useAlignment));
    sensorPlane->setUV(SensorInfo.vectorToGlobal(uVec, m_useAlignment), SensorInfo.vectorToGlobal(vVec, m_useAlignment));

    //boost pointer (will be deleted automatically ?!?!?)
    genfit::SharedPlanePtr sensorPlaneSptr(sensorPlane);

    // do extrapolation
    gfTrackState.extrapolateToPlane(sensorPlaneSptr);
  } catch (genfit::Exception& gfException) {
    B2WARNING("Fitting failed: " << gfException.getExcString());
    isgood = false;
    return intersec;
  } catch (...) {
    B2WARNING("Fitting failed: for some reason");
    isgood = false;
    return intersec;
  }

  //local position
  intersec = SensorInfo.pointToLocal(gfTrackState.getPos(), m_useAlignment);

  //try to get the momentum
  B2DEBUG(1, "Fitted momentum on the plane p = " << gfTrackState.getMom().Mag());

  // no tolerance currently! Maybe one should be added!
  double tolerance = 0.0;
  bool inside = SensorInfo.inside(intersec.X(), intersec.Y(), tolerance, tolerance);

  // get intersection point in local coordinates with covariance matrix
  TMatrixDSym covMatrix = gfTrackState.getCov(); // 5D with elements q/p,u',v',u,v in plane system

  // get ROI by covariance matrix and local intersection point
  du = std::sqrt(covMatrix(3, 3));
  dv = std::sqrt(covMatrix(4, 4));

  if (inside) isgood = true;

  return intersec;
}


void PXDOccupancyModule::defineHisto()
{
  // deleting all histograms and trees should be handled by the HistoModule !?
  // Correct me if I am wrong because otherwise this is a nice Memory leak!
  m_tree = new TTree("PXDClusterTree", "Tree for calculating pxd efficiencies");

  // assign initial values for all branches
  m_u_fit = 99;
  m_v_fit = 99;
  m_ucell_fit = -1;
  m_vcell_fit = -1;
  m_sigma_u_fit = -1;
  m_sigma_v_fit = -1;
  m_layernumber = -1;
  m_laddernumber = -1;
  m_sensornumber = -1;
  m_badcluster = 1;
  m_fit_pValue = -1;
  m_fit_mom = -1;
  m_fit_theta = -9999;
  m_fit_phi = -9999;
  m_fit_ndf = -1;
  m_event = -1;
  m_run = -1;
  m_subrun = -1;
  m_charge = -99;
  m_charge_pdg = -1;
  m_track_matched = 0;
  m_u_clus = -9999;
  m_v_clus = -9999;
  m_clus_charge = -9999;
  m_clus_seedcharge = -9999;
  m_clus_size = -9999;
  m_clus_usize = -9999;
  m_clus_vsize = -9999;
  m_nsvdtracks = -1;


  // define all tree branches
  m_tree->Branch("u_clus", &m_u_clus, "u_clus/D");
  m_tree->Branch("v_clus", &m_v_clus, "v_clus/D");
  m_tree->Branch("clus_charge", &m_clus_charge, "clus_charge/I");
  m_tree->Branch("clus_seedcharge", &m_clus_seedcharge, "clus_seedcharge/I");
  m_tree->Branch("clus_size", &m_clus_size, "clus_size/I");
  m_tree->Branch("clus_usize", &m_clus_usize, "clus_usize/I");
  m_tree->Branch("clus_vsize", &m_clus_vsize, "clus_vsize/I");
  m_tree->Branch("u_fit", &m_u_fit, "u_fit/D");
  m_tree->Branch("v_fit", &m_v_fit, "v_fit/D");
  m_tree->Branch("sigma_u_fit", &m_sigma_u_fit, "sigma_u_fit/D");
  m_tree->Branch("sigma_v_fit", &m_sigma_v_fit, "sigma_v_fit/D");
  m_tree->Branch("layer", &m_layernumber, "layer/I");
  m_tree->Branch("ladder", &m_laddernumber, "ladder/I");
  m_tree->Branch("sensor", &m_sensornumber, "sensor/I");
  m_tree->Branch("badcluster", &m_badcluster, "m_badcluster/I");
  m_tree->Branch("ucell_fit", &m_ucell_fit, "ucell_fit/I");
  m_tree->Branch("vcell_fit", &m_vcell_fit, "vcell_fit/I");
  m_tree->Branch("event", &m_event, "event/I");
  m_tree->Branch("run", &m_run, "run/I");
  m_tree->Branch("subrun", &m_subrun, "subrun/I");
  m_tree->Branch("fit_pValue", &m_fit_pValue, "fit_pValue/D");
  m_tree->Branch("fit_mom", &m_fit_mom, "fit_mom/D");
  m_tree->Branch("fit_theta", &m_fit_theta,  "fit_theta/D");
  m_tree->Branch("fit_phi", &m_fit_phi, "fit_phi/D");
  m_tree->Branch("fit_ndf", &m_fit_ndf, "fit_ndf/I");
  m_tree->Branch("track_matched", &m_track_matched, "track_matched/I");
  m_tree->Branch("charge", &m_charge, "charge/D");
  m_tree->Branch("charge_pdg", &m_charge_pdg, "charge_pdg/I");


  m_eventtree = new TTree("PXDEventTree", "Tree for calculating pxd efficiencies");
  m_eventtree->Branch("nsvdtracks", &m_nsvdtracks, "nsvdtracks/I");
  m_eventtree->Branch("event", &m_event, "event/I");
  m_eventtree->Branch("event_DHC", &m_pxdTriggerNr, "event_DHC/I");
  m_eventtree->Branch("event_HLT", &m_triggerNr, "event_HLT/I");



}




int
PXDOccupancyModule::findClosestTrack(VxdID& avxdid, TVector3 clusterpos)
{
  int closest = -1;
  double mindist = 999999999999; //definetly outside of the sensor

  VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
  if (info.getType() != VXD::SensorInfoBase::PXD) return closest;

  StoreArray<genfit::Track> tracks(m_tracksname);

  //loop the tracks
  for (int itrk = 0; itrk < tracks.getEntries(); itrk++) {

    bool isgood = false;
    double sigu(-9999);
    double sigv(-9999);
    TVector3 intersection = getTrackInterSec(info, tracks[itrk], isgood, sigu, sigv);

    //2D dist sqared
    double dist = (clusterpos - intersection).Mag();
    if (dist < mindist && isgood) {
      closest = itrk;
      mindist = dist;
    }
  }

  return closest;

}

bool PXDOccupancyModule::getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Trigger Shifter --> invalid packet size (32bit words) " << hex << px.size());
    return false;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());


  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    return false;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! " << hex << data[0]);
    return false;
  }

  Frames_in_event = ((ubig32_t*)data.data())[1];
  if (Frames_in_event < 1 || Frames_in_event > 250) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    return false;
  }

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes
    lo = ((ubig32_t*)tableptr)[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      return false;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      return false;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      if (unpack_dhc_frame(ll + (char*)dataptr, innerDHH, outerHLT)) return true;
      ll += lo; /// no rounding needed
    }
  }
  return false;
}

bool PXDOccupancyModule::unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT)
{
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1 //CLUSTER FRAME
#define DHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_START   0x3
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_END     0x4
// DHC envelope, new
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC
// Onsen processed data, new
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP     0xD
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE     0x9
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI     0xF
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG     0xE

  switch (((*(ubig16_t*)data) & 0x7800) >> 11) {
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_START: {
      innerDHH = ((ubig16_t*)data)[1];
      return true;
    }
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_START: {
      // workaround for BonnDAQ ...
      innerDHH = ((ubig16_t*)data)[1];
      return true;
    }
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG: {
      outerHLT = ((ubig16_t*)data)[1];
      //outerHLT = ((ubig32_t*)data)[2];
      break;
    }
    default:
      break;

  }
  return false;
}

