/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/


//fw-stuff
#include "tracking/modules/VXDTFHelperTools/TFAnalizerModule.h"
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/gearbox/Const.h>



//C++-stuff
#include <iostream>
#include <iomanip> // for setprecision on cout-doubles
#include <string>
#include <functional>


//stl-stuff
#include <map>
#include <algorithm>
#include <utility>

//root-stuff
#include <TMath.h>

//boost stuff
#include <boost/foreach.hpp>
#include <boost/math/special_functions/sign.hpp>

using namespace std;
using namespace Belle2;
using boost::tuple;
using boost::math::sign;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TFAnalizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TFAnalizerModule::TFAnalizerModule() : Module()
{
  //Set module properties
  setDescription("analyzes quality of cell-o-mat versus mcTrackFinder");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("fileExportMcTracks", m_PARAMFileExportMcTracks, "export mc Trackfinder tracks into file", bool(true));
  addParam("fileExportTfTracks", m_PARAMFileExportTfTracks, "export vxd Trackfinder tracks into file", bool(true));
  addParam("mcTCname", m_PARAMmcTCname, "special name for mcTF track candidates", string("mcTracks"));
  addParam("caTCname", m_PARAMcaTCname, "special name for caTF track candidates", string(""));
  addParam("qiThreshold", m_PARAMqiThreshold, " chose value to filter TCs found by VXDTF. TCs having QIs lower than this value won't be marked as reconstructed", double(0.7));
  addParam("minNumOfHitsThreshold", m_PARAMminNumOfHitsThreshold, " defines how many hits of current TC has to be found again to be accepted as recovered, standard is 3 hits", int(3));
  addParam("printExtentialAnalysisData", m_PARAMprintExtentialAnalysisData, "set true, if you want to cout special Info to the shell", bool(false));

  addParam("minTMomentumFilter", m_PARAMminTMomentumFilter, "to narrow down the relevant mcTracks, this minFilter can be set to filter tracks having lower transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges", double(0.));
  addParam("maxTMomentumFilter", m_PARAMmaxTMomentumFilter, "to narrow down the relevant mcTracks, this maxFilter can be set to filter tracks having higher transverse momentum than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges", double(5.));

}


TFAnalizerModule::~TFAnalizerModule()
{

}

void TFAnalizerModule::initialize()
{
  StoreArray<GFTrackCand>::required(m_PARAMmcTCname);
  StoreArray<GFTrackCand>::required(m_PARAMcaTCname);
//  B2WARNING("TFAnalizerModule: at the moment, no curling tracks are supported! When you feed this module with curling tracks, results can be wrong and misleading")
  B2WARNING("TFAnalizerModule: at the moment, trapezoidal sensors are not supported. using Thetas 17-50° will produce wrong results!")
  m_countReconstructedTCs = 0;
  m_eventCounter = 0;
  m_mcTrackCounter = 0;
  m_caTrackCounter = 0;
  m_countedPerfectRecoveries = 0;
  m_mcTrackVectorCounter = 0;
  m_wrongChargeSignCounter = 0;
  m_totalRealHits = 0;
}


void TFAnalizerModule::beginRun()
{

}


void TFAnalizerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(1, "################## entering TFAnalizer - event " << m_eventCounter << " ######################");



  /// import all GFTrackCands (McFinder, TFinder)
  StoreArray<GFTrackCand> mcTrackCandidates(m_PARAMmcTCname);
  StoreArray<GFTrackCand> caTrackCandidates(m_PARAMcaTCname);
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;

  StoreArray<PXDTrueHit> pxdTrueHits;
  StoreArray<SVDTrueHit> svdTrueHits;
//  StoreArray<McParticle> mcParticles;
  RelationIndex<PXDCluster, PXDTrueHit> relPXDCluster2TrueHit; /* <FROM, TO> */
  RelationIndex<SVDCluster, SVDTrueHit> relSVDCluster2TrueHit;
  RelationArray relPXDClusterTrueHit(pxdClusters, pxdTrueHits);
  RelationArray relSVDClusterTrueHit(svdClusters, svdTrueHits);

  StoreArray<VXDTFInfoBoard> extraInfos;

  int numOfMcTCs = mcTrackCandidates.getEntries();
  int numOfCaTCs = caTrackCandidates.getEntries();
//  int numOfInfoBoards = extraInfos.getEntries();

  m_mcTrackCounter += numOfMcTCs;
  m_caTrackCounter += numOfCaTCs;

  vector<VXDTrackCandidate> caTcVector;
  vector<VXDTrackCandidate> mcTcVector;



  /// get info needed for comparison and coord-export:
  B2DEBUG(1, "importing " << numOfMcTCs << " mcTrackCandidates...")

  for (int i = 0; i not_eq numOfMcTCs; ++i) {
    B2DEBUG(10, "--importing trackCandidate " << i << "...")
    GFTrackCand* aTC =  mcTrackCandidates[i];
    extractHits(aTC, relPXDCluster2TrueHit, relSVDCluster2TrueHit, pxdClusters, svdClusters, extraInfos, mcTcVector, true, i);    /// extractHits
    /// missing: export2File!
  }
  m_mcTrackVectorCounter += mcTcVector.size();


  B2DEBUG(1, "importing " << numOfCaTCs << " caTrackCandidates...")
  for (int i = 0; i not_eq numOfCaTCs; ++i) {
    B2DEBUG(10, "--importing trackCandidate " << i << "...")
    GFTrackCand* aTC =  caTrackCandidates[i];
    extractHits(aTC, relPXDCluster2TrueHit, relSVDCluster2TrueHit, pxdClusters, svdClusters, extraInfos, caTcVector, false, i); /// extractHits
  }


  BOOST_FOREACH(VXDTrackCandidate & mcTC, mcTcVector) {
    BOOST_FOREACH(VXDTrackCandidate & caTC, caTcVector) {
      B2DEBUG(100, "-before checkCompatibility: caTC.indexNumber: " << caTC.indexNumber << ", caTC.qualityIndex: " << caTC.qualityIndex)
      B2DEBUG(100, "-run checkCompatibility for mcTC " << mcTC.indexNumber << " using caTC " << caTC.indexNumber)
      checkCompatibility(mcTC, caTC); /// checkCompatibility
      B2DEBUG(100, "-after checkCompatibility: caTC.indexNumber: " << caTC.indexNumber << ", caTC.qualityIndex: " << caTC.qualityIndex)
    }
  } // determine compatibility matrix

  vector<int> foundIDs;

  if (int(caTcVector.size()) != 0) {   // ! caTcVector.empty()
    B2DEBUG(1, " between loops: caTcVector.size():" << caTcVector.size() << ", caTcVector[0].indexNumber: " << caTcVector[0].indexNumber << ", finAssID: " << caTcVector[0].finalAssignedID << ", QI: " << caTcVector[0].qualityIndex)
  }
  BOOST_FOREACH(VXDTrackCandidate & caTC, caTcVector) {

    B2DEBUG(10, " caTC " << caTC.indexNumber << ": has got the following assigned mc trackCandidates: (best value: mcTCID: " << caTC.finalAssignedID << ", QI: " << caTC.qualityIndex << ")")
    BOOST_FOREACH(CompatibilityIndex thisEntry, caTC.compatiblePartners) {
      B2DEBUG(10, "	Partner: " << boost::get<0>(thisEntry) << ", shares " << boost::get<1>(thisEntry) << " hits, thisTC has got " << boost::get<2>(thisEntry) << " dirty hits, " << boost::get<3>(thisEntry) << " hits are only in partner and they have a qualityRelation of " << boost::get<4>(thisEntry))
    }
    B2DEBUG(10, "-------------------------------------------------------------------------------")
    if (caTC.finalAssignedID == -1) {  continue; }  // in this case, absolutely no hit of caTC is of any mcTC
    if (caTC.numOfCorrectlyAssignedHits < m_PARAMminNumOfHitsThreshold) {
      if (m_PARAMprintExtentialAnalysisData == true) { printInfo(0, mcTcVector[caTC.finalAssignedID], caTC); }
      continue;
    }

    int totalHits = caTC.numOfCorrectlyAssignedHits + caTC.numOfBadAssignedHits;
    double trackQuality = double(caTC.numOfCorrectlyAssignedHits) / double(totalHits);
    if (trackQuality < m_PARAMqiThreshold) {
      if (m_PARAMprintExtentialAnalysisData == true) { printInfo(0, mcTcVector[caTC.finalAssignedID], caTC); }
      continue;
    }


    bool doubleCountFlag = false;
    BOOST_FOREACH(int iD, foundIDs) { if (iD == caTC.finalAssignedID) { doubleCountFlag = true; } }
    if (doubleCountFlag == true) { continue; }  // no output here since it has been recovered several times, we simply take the first caTC (which is not necessarily the best one, but at least recovered)

    foundIDs.push_back(caTC.finalAssignedID);
    if (caTC.qualityIndex > 0.99) {
      if (m_PARAMprintExtentialAnalysisData == true) { printInfo(2, mcTcVector[caTC.finalAssignedID], caTC); }
      m_countedPerfectRecoveries++;
    } else {
      if (m_PARAMprintExtentialAnalysisData == true) { printInfo(1, mcTcVector[caTC.finalAssignedID], caTC); }
    }
  }


  std::sort(foundIDs.begin(), foundIDs.end());
  std::unique(foundIDs.begin(), foundIDs.end());
  int numOfFoundIDs = foundIDs.size();
  m_countReconstructedTCs += numOfFoundIDs;

  BOOST_FOREACH(VXDTrackCandidate & mcTC, mcTcVector) {
    bool foundFlag = false;
    BOOST_FOREACH(int iD, foundIDs) { if (iD == mcTC.indexNumber) { foundFlag = true; } }
    m_totalRealHits += int(mcTC.coordinates.size() * 0.5);
    if (foundFlag == true) {
      if (m_PARAMprintExtentialAnalysisData == true) { printMC(true, mcTC); } /// printMC
      m_totalRealHits += int(mcTC.coordinates.size() * 0.5);
    } else {
      if (m_PARAMprintExtentialAnalysisData == true) { printMC(false, mcTC); } /// printMC
    }
  } // print info about all found and lost mcTCs

  BOOST_FOREACH(VXDTrackCandidate & caTC, caTcVector) {
    if (caTC.finalAssignedID == -1 || caTC.qualityIndex < m_PARAMqiThreshold) {
      if (m_PARAMprintExtentialAnalysisData == true) { printCA(false, caTC); } /// printCA
    } else {
      if (m_PARAMprintExtentialAnalysisData == true) { printCA(true, caTC); } /// printCA
    }
  } // print info about all ghost and good caTCs


  B2DEBUG(1, "Event " << m_eventCounter << ": There are " << int(mcTcVector.size()) << " mcTCs, ")
  BOOST_FOREACH(int ID, foundIDs) {
    B2DEBUG(1, " - ID " << ID << " recovered")
  }
  B2DEBUG(1, " the tested TrackFinder found " << numOfFoundIDs << " IDs within " << int(caTcVector.size()) << " TCs and lost " << int(mcTcVector.size() - foundIDs.size()))

}


void TFAnalizerModule::endRun()
{

  B2INFO("TFAnalizerModule: After " << m_eventCounter + 1 << " events there was a total number of " << m_mcTrackCounter << " mcTrackCandidates and " << m_totalRealHits << " realHits. Of these TCs, " << m_mcTrackVectorCounter << " mcTrackCandidates where used for analysis because of cutoffs.")
  B2INFO("TFAnalizerModule: There were " << m_caTrackCounter << " caTrackCandidates, the VXDTF found (total/ideal)" << m_countReconstructedTCs << "/" << m_countedPerfectRecoveries << " TCs -> efficiency(total/ideal/ghost): " << double(100 * m_countReconstructedTCs) / double(m_mcTrackVectorCounter) << "%/" << double(100 * m_countedPerfectRecoveries) / double(m_mcTrackVectorCounter) << "/%" << double(100 * (m_caTrackCounter - m_countReconstructedTCs)) / double(m_countReconstructedTCs) << "%, number of times where charge was guessed wrong: " << m_wrongChargeSignCounter)


}


void TFAnalizerModule::terminate()
{

}





void TFAnalizerModule::printInfo(int recoveryState, VXDTrackCandidate& mcTC, VXDTrackCandidate& caTC)
{
  string tcType = "unknown tcType";
  if (recoveryState == 0) {   // not recovered
    tcType = "LostMCTC";
  } else if (recoveryState == 1) {   // contaminated TC
    tcType = "ContaminatedMCTC";
  } else if (recoveryState == 2) {
    tcType = "PerfectMCTC";
  }

  TVector3 zDir; // vector parallel to z-axis
  zDir.SetXYZ(0., 0., 1.);
  double residual = mcTC.pTValue - caTC.pTValue; // difference of estimated and real transverseMomentum;
  TVector3 caDirection = caTC.direction;
  TVector3 mcDirection = mcTC.direction;

  double theta = mcDirection.Angle(zDir);
  theta = theta * 180.*TMath::InvPi();
  double angle = caDirection.Angle(mcDirection); // angle between the initial momentum vectors in rad
  angle = angle * 180.*TMath::InvPi(); // ... and now in grad
  caDirection.SetZ(0.);
  mcDirection.SetZ(0.);
  double transverseAngle = caDirection.Angle(mcDirection);
  transverseAngle = transverseAngle * 180.*TMath::InvPi(); // ... and now in grad

  B2INFO("PRINTINFO: At event " << m_eventCounter <<
         ": mcType §" << tcType <<
         "§ having §" << mcTC.coordinates.size() <<
         "§ hits with theta of §" << setprecision(4) << theta <<
         "§° got pT of §" << setprecision(4) << mcTC.pTValue <<
         "§ GeV/c, assigned caTC got pT of §" << setprecision(4) << caTC.pTValue <<
         "§ GeV/c, and probValue of §" << setprecision(6) << caTC.probValue <<
         "§. Their residual of pT was §" << setprecision(4) << residual <<
         "§ GeV/c, their residual of angle was §" << setprecision(4) << angle <<
         "§ in grad, their residual of transverse angle was §" << setprecision(4) << transverseAngle <<
         "§ with PDGCode of mcTC: §" << mcTC.pdgCode <<
         "§ and PDGCode of caTC: §" << caTC.pdgCode <<
         "§") // '§' will be used to filter
  if (mcTC.pdgCode == 13 or mcTC.pdgCode == -13) {   // muons
    if (caTC.pdgCode == 13 or caTC.pdgCode == -13) {
      if (sign(caTC.pdgCode) not_eq sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    } else {
      if (sign(caTC.pdgCode) == sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    }

  } else {
    if (caTC.pdgCode == 13 or caTC.pdgCode == -13) {
      if (sign(caTC.pdgCode) == sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    } else {
      if (sign(caTC.pdgCode) not_eq sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    }
  }

}


void TFAnalizerModule::printMC(bool type, VXDTrackCandidate& mcTC)
{
  string info;
  if (type == true) { info = "FOUNDINFO"; } else { info = "LOSTINFO"; }

  TVector3 zDir; // vector parallel to z-axis
  zDir.SetXYZ(0., 0., 1.);
  TVector3 mcDirection = mcTC.direction;

  double theta = mcDirection.Angle(zDir);
  theta = theta * 180.*TMath::InvPi();
  double distVertex2Zero = mcTC.vertex.Mag();
  double distTVertex2Zero = mcTC.vertex.Perp();
  double distZVertex2Zero = mcTC.vertex.Z();
  int pdg = mcTC.pdgCode;

  B2INFO(info << ": At event " << m_eventCounter <<
         ": MC with ID " << mcTC.indexNumber << " having §" << mcTC.coordinates.size() <<
         "§ hits with theta of §" << setprecision(4) << theta <<
         "§° got pT of §" << setprecision(4) << mcTC.pTValue <<
         "§ GeV/c and vertex distance to origin: §" << setprecision(4) << distVertex2Zero <<
         "§cm, transverseDistance: §" << setprecision(4) << distTVertex2Zero <<
         "§cm, zDistance: §" << setprecision(4) << distZVertex2Zero <<
         "§, and pdg of: §" << setprecision(4) << pdg <<
         "§") // '§' will be used to filter
}



void TFAnalizerModule::printCA(bool type, VXDTrackCandidate& caTC)
{
  string info;
  if (type == true) { info = "FOUNDCATCINFO"; } else { info = "LOSTCATCINFO"; }

  TVector3 zDir; // vector parallel to z-axis
  zDir.SetXYZ(0., 0., 1.);
  TVector3 caDirection = caTC.direction;

  double theta = caDirection.Angle(zDir);
  theta = theta * 180.*TMath::InvPi();
  int pdg = caTC.pdgCode;

  B2INFO(info << ": At event " << m_eventCounter <<
         ": CA with assigned ID " << caTC.finalAssignedID <<
         " having §" << caTC.coordinates.size() <<
         "§ hits with theta of §" << setprecision(4) << theta <<
         "§° got pT of §" << setprecision(4) << caTC.pTValue <<
         "§ GeV/c, QI of §" << setprecision(4) << caTC.qualityIndex << "§ and pdg of: " << pdg) // '§' will be used to filter
}



void TFAnalizerModule::extractHits(GFTrackCand* aTC,
                                   RelationIndex<PXDCluster, PXDTrueHit>& relationPXD,
                                   RelationIndex<SVDCluster, SVDTrueHit>& relationSVD,
                                   StoreArray<PXDCluster>& pxdClusters,
                                   StoreArray<SVDCluster>& svdClusters,
                                   StoreArray<VXDTFInfoBoard>& infoBoards,
                                   vector<VXDTrackCandidate>& tcVector,
                                   bool isMCTC,
                                   int index)
{
  TVector3 hitLocal, hitGlobal;
  VxdID aVxdID;
  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  B2DEBUG(10, "starting extractHits... isMCTC: " << isMCTC << ", index: " << index)
  int numOfHits = aTC->getNHits();
  B2DEBUG(10, " found " << numOfHits << " hits for TC " << index)
  vector<int> pxdHitIDsOfCurrentTC;
  vector<int> svdHitIDsOfCurrentTC;
  vector<TVector3> coordinates;
//  vector<PXDTrueHit*> pxdTrueHits;
  vector<const SVDTrueHit*> svdTrueHits;

  typedef std::map<int, int> map_type;
  typedef pair<unsigned int, vector<int> > pair4SvdHitMap;
  map<unsigned int, vector<int>  > svdHitMap; // since svdClusters are only 1D and we can not assume, that two clusters of the same hit are found in pairs within the TC-hit-List, we have to sort them before using them
  for (int hitIndex = 0; hitIndex not_eq numOfHits; ++hitIndex) {
    B2DEBUG(100, "----importing hit " << hitIndex << " from trackCandidate...")
    int detID = -1; // ID of detector
    int hitID = -1; // ID of Hit in StoreArray
    aTC->getHit(hitIndex, detID, hitID); // sets detId and hitId for given hitIndex
    B2DEBUG(100, "----got Hitinfo. detID: " << detID << ", hitID: " << hitID)
    if (detID == Const::PXD) { // pxd
      pxdHitIDsOfCurrentTC.push_back(hitID);
      hitLocal.SetXYZ(pxdClusters[hitID]->getU(), pxdClusters[hitID]->getV(), 0);
      B2DEBUG(100, "gotPXD, u: " << pxdClusters[hitID]->getU() << ", v: " << pxdClusters[hitID]->getV())
      aVxdID = pxdClusters[hitID]->getSensorID();
      VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
      hitGlobal = aSensorInfo.pointToGlobal(hitLocal);
      coordinates.push_back(hitGlobal);
    }
    if (detID == Const::SVD) { // svd
      svdHitIDsOfCurrentTC.push_back(hitID);
      RelationIndex<SVDCluster, SVDTrueHit>::range_from relationRange = relationSVD.getElementsFrom(svdClusters[hitID]);
      while (relationRange.first not_eq relationRange.second) {
        const SVDTrueHit* aTrueHit = relationRange.first->to;

        aVxdID = aTrueHit->getSensorID();

        hitLocal.SetXYZ(aTrueHit->getU(), aTrueHit->getV(), 0);
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
        hitGlobal = aSensorInfo.pointToGlobal(hitLocal);
        coordinates.push_back(hitGlobal);
        B2DEBUG(100, "gotSVD, u: " << aTrueHit->getU() << ", v: " << aTrueHit->getV())

        svdTrueHits.push_back(aTrueHit);
        ++relationRange.first;
      }
    }
  } /// getting Info for each hit. 1D clusters make the whole thing complicated since it has to be checked, whether two 1D hits form a 2D hit of the track or not

  TcInfoTuple tcInfo = boost::make_tuple(pxdHitIDsOfCurrentTC, svdHitIDsOfCurrentTC, coordinates, svdTrueHits);

  ///get Momentum information:
  TVectorD stateSeed = aTC->getStateSeed();
//  TVector3 dirSeed = aTC->getDirSeed();
//  double qOverP = aTC->getQoverPseed();
//  double pOverQ = 1./qOverP;
  TVector3 momentum, momentum_t, vertex; // = pOverQ*dirSeed;
  momentum[0] = stateSeed(3); momentum[1] = stateSeed(4); momentum[2] = stateSeed(5);
  vertex[0] = stateSeed(0); vertex[1] = stateSeed(1); vertex[2] = stateSeed(2);
  double pValue = momentum.Mag();
  momentum_t = momentum;
  momentum_t.SetZ(0.);
  double pT = momentum_t.Mag();
  int pdgCode = aTC->getPdgCode();
  bool gotNewMomentum = false;

  if (isMCTC == true) {   // want momentum vector of innermost hit, not of primary vertex

    if (pT < m_PARAMminTMomentumFilter or pT > m_PARAMmaxTMomentumFilter) {
      return; /// do not store mcTrack in this case
    }

    int detID = -1; // ID of detector
    int hitID = -1; // ID of Hit in StoreArray
    TVector3 tempMomentum; // momentum of innermost hit
    aTC->getHit(0, detID, hitID); // 0 means innermost hit
    if (detID == Const::PXD) {  // means PXD
      RelationIndex<PXDCluster, PXDTrueHit>::range_from relationRange = relationPXD.getElementsFrom(pxdClusters[hitID]);
      while (relationRange.first not_eq relationRange.second) {
        // since more than one trueHit can be the cause of current hit, we have to find the real TrueHit. Identified by |momentum|
        const PXDTrueHit* aTrueHit = relationRange.first->to;
        tempMomentum = aTrueHit->getMomentum();

        aVxdID = aTrueHit->getSensorID();
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
        tempMomentum = aSensorInfo.vectorToGlobal(tempMomentum);

        if (pValue - tempMomentum.Mag() < pValue * 0.1) { gotNewMomentum = true; break; } // if difference in Momentum is less than 10% of initial momentum, we accept current value as the real one

        ++relationRange.first;
      }
    } else if (detID == Const::SVD) {  // SVD
      RelationIndex<SVDCluster, SVDTrueHit>::range_from relationRange = relationSVD.getElementsFrom(svdClusters[hitID]);
      while (relationRange.first not_eq relationRange.second) {
        // since more than one trueHit can be the cause of current hit, we have to find the real TrueHit. Identified by |momentum|
        const SVDTrueHit* aTrueHit = relationRange.first->to;
        tempMomentum = aTrueHit->getMomentum();

        aVxdID = aTrueHit->getSensorID();
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
        tempMomentum = aSensorInfo.vectorToGlobal(tempMomentum);

        if (pValue - tempMomentum.Mag() < pValue * 0.1) { gotNewMomentum = true; break; } // if difference in Momentum is less than 10% of initial momentum, we accept current value as the real one

        ++relationRange.first;
      }
    } else { B2FATAL("TFAnalizer - this track candidate does not have any VXD-hits, can not analyze it") }

    if (gotNewMomentum == true) {
      momentum = tempMomentum;
      pValue = momentum.Mag();
      pT = momentum.Pt();
      B2DEBUG(10, "TFAnalizer event " << m_eventCounter << ": calculated new momentum (pT = " << pT << ")");
    }
  }

  VXDTrackCandidate newTC;
  newTC.pxdClusterIDs = boost::get<0>(tcInfo);
  newTC.svdClusterIDs = boost::get<1>(tcInfo);
  newTC.coordinates = boost::get<2>(tcInfo);
  newTC.direction = momentum;
  newTC.svdTrueHits = boost::get<3>(tcInfo);
  newTC.isMCtrackCandidate = isMCTC;
  newTC.indexNumber = index;
  newTC.pTValue = pT;
  newTC.pValue = pValue;
  newTC.pdgCode = pdgCode;

  if (isMCTC == true) {
    newTC.finalAssignedID = tcVector.size();
    newTC.qualityIndex = 1;
    newTC.vertex = vertex;
  } else {
    newTC.finalAssignedID = -1;
    newTC.qualityIndex = 0;

    /// read additional info:
    int gfIndex = aTC->getMcTrackId();
    newTC.probValue = infoBoards[gfIndex]->getProbValue();
    newTC.survivedFit = infoBoards[gfIndex]->isFitPossible();
  }
  B2DEBUG(10, " end of extractHits. TC isMCTC: " << isMCTC << ", PDGCode: " << pdgCode << ", finalAssignedID: " << newTC.finalAssignedID << ", indexNumber: " << newTC.indexNumber << ", pValue: " << pValue << ", pT: " << pT)
  tcVector.push_back(newTC);
}





void TFAnalizerModule::checkCompatibility(VXDTrackCandidate& mcTC, VXDTrackCandidate& caTC)
{
  double compatibilityValue;
  int goodHitsInCaTC = 0; // if hit of mcTC is found in caTC too -> ++;
  int badHitsInCaTC = 0; // if hit of caTC is not found in mcTC -> ++:
  B2DEBUG(100, "--checkCompatibility has started, comparing mcTC: " << mcTC.indexNumber << " with caTC: " << caTC.indexNumber)
  /** mehrere Ansatzpunkte: 1.
   * 1. caTC-based: wie viele seiner Hits waren echt?
   *  A. rechne aus, wie viele Hits er hatte.
   *  B. concatenate hits von beiden, unique und sort them.
   *  C. vergleiche Größe von gesamthits, vor und nach unique. Relation zwischen Differenz der beiden und der numOfCATCHits -> compatibilityValue caTC
   *  D. Differenz der beiden von der gesamtZahl an Hits vom CATC
   * 2. mcTC-Based:  Differenz der beiden von der gesamtZahl an Hits vom MCTC
   *
   * beide den jeweiligen anderen Indexwert zuweisen, falls kompatibel.
   */
  BOOST_FOREACH(int indexValueMC, mcTC.pxdClusterIDs) {    /// PXDHits
    int trueCtr = 0, falseCtr = 0;
    BOOST_FOREACH(int indexValueCA, caTC.pxdClusterIDs) {
      if (indexValueCA == indexValueMC) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr != 0) { goodHitsInCaTC++; }
  } // check good hits for mcTC (PXD)
  B2DEBUG(10, "--after check good hits for mcTC(PXD), value is: " << goodHitsInCaTC)

  BOOST_FOREACH(const SVDTrueHit * aTrueHitmc, mcTC.svdTrueHits) {   /// SVDHits
    int trueCtr = 0, falseCtr = 0;
    BOOST_FOREACH(const SVDTrueHit * aTrueHitca, caTC.svdTrueHits) {
      if (aTrueHitca == aTrueHitmc) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr != 0) { goodHitsInCaTC++; }
  } // check good hits for mcTC (SVD)
  B2DEBUG(10, "--after check good hits for mcTC(SVD), value is: " << goodHitsInCaTC)

  BOOST_FOREACH(int indexValueCA, caTC.pxdClusterIDs) {
    int trueCtr = 0, falseCtr = 0;
    BOOST_FOREACH(int indexValueMC, mcTC.pxdClusterIDs) {
      if (indexValueCA == indexValueMC) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr == 0) { badHitsInCaTC++; }
  } // check bad hits for caTC (PXD)
  B2DEBUG(10, "--after check bad hits for caTC (PXD), value is: " << badHitsInCaTC)

  BOOST_FOREACH(const SVDTrueHit * aTrueHitca, caTC.svdTrueHits) {   /// SVDHits
    int trueCtr = 0, falseCtr = 0;
    BOOST_FOREACH(const SVDTrueHit * aTrueHitmc, mcTC.svdTrueHits) {
      if (aTrueHitca == aTrueHitmc) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr == 0) { badHitsInCaTC++; }
  } // check bad hits for caTC (SVD)

  B2DEBUG(10, "--after check bad hits for caTC (SVD), value is: " << badHitsInCaTC)
  int totalMCHits = mcTC.pxdClusterIDs.size() + mcTC.svdTrueHits.size();
  compatibilityValue = double(goodHitsInCaTC) / double(totalMCHits);
  B2DEBUG(10, "calculated compatibilityValue: " << compatibilityValue << ", caTC.qualityIndex: " << caTC.qualityIndex)

  if (caTC.qualityIndex < compatibilityValue) {  // in this case, the current mcTC suits better for the caTC guess than for the last one.
    caTC.qualityIndex = compatibilityValue;
    caTC.finalAssignedID = mcTC.finalAssignedID;
    caTC.numOfCorrectlyAssignedHits = goodHitsInCaTC;
    caTC.numOfBadAssignedHits = badHitsInCaTC;
  }
  B2DEBUG(10, "after check, caTC.qualityIndex: " << caTC.qualityIndex << ", caTC.finalAssignedID: " << caTC.finalAssignedID)
  CompatibilityIndex caCindex = boost::make_tuple(mcTC.indexNumber, goodHitsInCaTC, badHitsInCaTC, totalMCHits - goodHitsInCaTC, compatibilityValue);
  CompatibilityIndex mcCindex = boost::make_tuple(caTC.indexNumber, goodHitsInCaTC, totalMCHits - goodHitsInCaTC, badHitsInCaTC, compatibilityValue);
  mcTC.compatiblePartners.push_back(mcCindex);
  caTC.compatiblePartners.push_back(caCindex);
}
