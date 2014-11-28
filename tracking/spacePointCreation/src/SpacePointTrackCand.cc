/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// framework
#include <framework/datastore/StoreArray.h>
// #include <testbeam/vxd/dataobjects/TelCluster.h> // for completeness (in case someone wants to use telClusters) PLUS won't be neede in the future
#include <framework/gearbox/Const.h>

// testbeam SpacePoints
// #include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h> // not allowed to use in this class -> outsource constructor to module (done already)

// SpacePointTrackCand.h
#include <tracking/spacePointCreation/SpacePointTrackCand.h>


#include <TDatabasePDG.h>
#include <TVectorD.h>
#include <TMatrixD.h>

// debugging
#include <iostream>
#include <sstream>
#include <algorithm> // sort & unique

using namespace std;
using namespace Belle2;

ClassImp(SpacePointTrackCand)

SpacePointTrackCand::SpacePointTrackCand() :
  m_pdg(0),
  m_MCTrackID(-1),
  m_state6D(6),
  m_cov6D(6),
  m_q(0)
{

}

SpacePointTrackCand::SpacePointTrackCand(const std::vector<const Belle2::SpacePoint*> spacePoints, int pdgCode, double charge, int mcTrackID) :
  m_state6D(6),
  m_cov6D(6)
{
  m_pdg = pdgCode;
  m_q = charge;
  m_MCTrackID = mcTrackID;

  for (const SpacePoint * spacePoint : spacePoints) {
    m_trackSpacePoints.push_back(spacePoint);
  }
}

// SpacePointTrackCand::SpacePointTrackCand(const genfit::TrackCand& genfitTC) :
//   // have to initialize them like this, otherwise the assignment below does not work (ROOT needs to know the dimension beforehand)
//   m_state6D(6),
//   m_cov6D(6)
// {
// //   B2WARNING("You are calling a constructor which is not yet tested!"); // it is sort of being tested right now
//   B2WARNING("Calling a deprecated constructor: SpacePointTrackCand::SpacePoinTrackCand(const genfit::TrackCand& )")
//
//   B2DEBUG(30, "Generating SpacePointTrackCand from genfit::TrackCand");
//
//   m_pdg = genfitTC.getPdgCode();
//   m_q = genfitTC.getChargeSeed();
//   m_MCTrackID = genfitTC.getMcTrackId();
//   m_state6D = genfitTC.getStateSeed();
//   m_cov6D = genfitTC.getCovSeed();
//
// // These are probably not even needed here, leave them commented out for the moment
// //     StoreArray<TBSpacePoint> tbspacePoints;
// //     StoreArray<SpacePoint> spacePoints("pxdOnly");
// //     StoreArray<SpacePoint> spacePoints("couplesAllowed");
//
//   // more complicated part (for now doing it via the StoreArray and the Relations therein)
//   StoreArray<PXDCluster> pxdClusters;
//   StoreArray<SVDCluster> svdClusters;
// //     StoreArray<TelCluster> telClusters;
//
//   std::vector<HitInfo<PXDCluster> > tcPXDClusters; // all PXDClusters of the trackCandidate
//   std::vector<HitInfo<SVDCluster> > tcSVDClusters; // same for SVDCluster
// //     std::vector<HitInfo<TelCluster> > tcTelClusters; // ...
//
//   std::vector<HitInfo<SpacePoint> > tcSpacePoints; // need this as a intermediate step for sorting out double SpacePoint
//
//   // loop over all hits from the genfit::TrackCand and search for the Clusters in their StoreArrays
//   for (unsigned int iTCHit = 0; iTCHit < genfitTC.getNHits(); ++iTCHit) {
//     genfit::TrackCandHit* aTCHit = genfitTC.getHit(iTCHit);
//
//     int detID = aTCHit->getDetId();
//     int hitID = aTCHit->getHitId();
//     int planeID = aTCHit->getPlaneId();
//     B2DEBUG(60, "TrackCandHit " << iTCHit << " has detID: " << detID << ", hitID: " << hitID << ", planeID: " << planeID);
//
//     // check to which detector the hit belongs and add the according cluster
//     if (detID == Const::PXD) {
//       const PXDCluster* aCluster = pxdClusters[hitID];
//       tcPXDClusters.push_back({iTCHit, aCluster});
//       B2DEBUG(60, "Added PXDCluster " << aCluster->getArrayIndex() << " from StoreArray " << aCluster->getArrayName() << " with hitID " << hitID << " to tcPXDClusters");
//     } else if (detID == Const::SVD) {
//       const SVDCluster* aCluster = svdClusters[hitID];
//       tcSVDClusters.push_back({iTCHit, aCluster});
//       B2DEBUG(60, "Added SVDCluster " << aCluster->getArrayIndex() << " from StoreArray " << aCluster->getArrayName() << " with hitID " << hitID << " to tcSVDClusters");
//     } /*else if ( detID == Const::TEST ) {
//   const TelCluster* aCluster = telClusters[hitID];
//   tcTelClusters.push_back({iTCHit,aCluster});
//   B2DEBUG(60,"Added TelCluster " << aCluster->getArrayIndex() << " with hitID " << hitID << " to tcTelClusters");
//       } */else {
//       throw Unsupported_Det_Type();
//     }
//   }// for demonstration purposes (to see what happens next) commented out the throwing of the exception below
//
//   // PXDCluster SpacePoints
//   for (auto aPXDCluster : tcPXDClusters) {
//     // for checking how many relations exist
//     // RelationVector<SpacePoint> relVec = aPXDCluster.second->getRelationsFrom<SpacePoint>("ALL");
//     // B2DEBUG(60,"Found " << relVec.size() << " Relations with SpacePoint");
//     const SpacePoint* spacePoint = aPXDCluster.second->getRelatedFrom<SpacePoint>("ALL");
//     if (spacePoint == NULL) {
//       B2WARNING("Found no relation to SpacePoint for PXDCluster " << aPXDCluster.second->getArrayIndex() << " in StoreArray " << aPXDCluster.second->getArrayName() << ". This hit will be skipped and will not be contained in the SpacePointTrackCand!");
//       continue;
//     }
//     tcSpacePoints.push_back({aPXDCluster.first, spacePoint});
//     B2DEBUG(60, "Added SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " to tcSpacePoints");
//   }
//
//   // SVDCluster SpacePoints
//   for (auto aSVDCluster : tcSVDClusters) {
//     // for checking how many relations exist
//     // RelationVector<SpacePoint> relVec = aSVDCluster.second->getRelationsFrom<SpacePoint>("ALL");
//     // B2DEBUG(60,"Found " << relVec.size() << " relations with SpacePoint for SVDCluster");
//     const SpacePoint* spacePoint = aSVDCluster.second->getRelatedFrom<SpacePoint>("ALL");
//     if (spacePoint == NULL) {
//       B2WARNING("Found no relation to SpacePoint for SVDCLuster " << aSVDCluster.second->getArrayIndex() << " in StoreArray " << aSVDCluster.second->getArrayName() << ". This hit will be skipped and will not be contained in the SpacePointTrackCand!");
//       continue;
//     }
//     tcSpacePoints.push_back({aSVDCluster.first, spacePoint});
//     B2DEBUG(60, "Added SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " to tcSpacePoints");
//   }
//
//   // after adding the hits sensor-wise to tcSpacePoints order them after their appearance in the original genfit::TrackCand
//   std::sort(tcSpacePoints.begin(), tcSpacePoints.end(), [](const HitInfo<SpacePoint> aSP, const HitInfo<SpacePoint> bSP) {return aSP.first < bSP.first; });
//
//   int oldSize = tcSpacePoints.size();
//   auto newEnd = std::unique(tcSpacePoints.begin(), tcSpacePoints.end(), [](const HitInfo<SpacePoint> aSP, const HitInfo<SpacePoint> bSP) { return aSP.second == bSP.second; });
//   tcSpacePoints.resize(std::distance(tcSpacePoints.begin(), newEnd));
//
//   int newSize = tcSpacePoints.size();
//   B2DEBUG(70, "Size of tcSpacePoints before/after unique and resize: " << oldSize << "/" << newSize);
//
//   // Add SpacePoints to m_trackSpacePoints (ordered from inner -> outer)
//   for (auto aSpacePoint : tcSpacePoints) {
//     const SpacePoint* spacePoint = aSpacePoint.second;
//     m_trackSpacePoints.push_back(spacePoint);
//     m_trackSpacePointIndices.push_back(spacePoint->getArrayIndex());
//   }
// }

// destructor
// TODO: get segmentation fault here at the moment!
// removing code seems to work, but I have no idea why
SpacePointTrackCand::~SpacePointTrackCand()
{
//   for (unsigned int i=0; i<m_trackSpacePoints.size(); ++i) {
// //     std::cout << i << " of " << m_trackSpacePoints.size() << std::endl;
//     delete m_trackSpacePoints[i];
//   }
//   m_trackSpacePoints.clear();
}

// converting a SPTrackCand to a GFTrackCand
// const genfit::TrackCand SpacePointTrackCand::getGenFitTrackCand() const
// {
//   genfit::TrackCand returnTrackCand;
//   B2WARNING("WARNING SpacePointTrackCand::getGenFitTrackCand() is deprecated and will probably not be supported any longer."); // getting tested at the moment
//
//   const TVectorD stateSeed = this->getStateSeed();
//   const int pdgCode = this->getPdgCode();
//   const TMatrixDSym covSeed = this->getCovSeed();
//
//   returnTrackCand.set6DSeed(stateSeed, pdgCode);
//   returnTrackCand.setCovSeed(covSeed);
//
//   // fill track candidate with hits
//   for (const SpacePoint * aHit : this->getHits()) {
//     auto detType = aHit->getType();
//     // PXD SpacePoints
//     if (detType == VXD::SensorInfoBase::PXD) {
//       RelationVector<PXDCluster> pxdClusters = aHit->getRelationsTo<PXDCluster>("ALL");
//       B2DEBUG(70, "Found " << pxdClusters.size() << " relations to PXDcluster for this SpacePoint");
//       if (pxdClusters.size() == 0) {
//         B2WARNING("Could not find a relation to a PXDCluster for SpacePoint " << aHit->getArrayIndex() << " from StoreArray " << aHit->getArrayName() << ". The Hit belonging to this SpacePoint will not be in the genfit::TrackCand.");
//         continue;
//       }
//       for (const PXDCluster & aCluster : pxdClusters) {
//         int hitID = aCluster.getArrayIndex();
//         returnTrackCand.addHit(Const::PXD, hitID);
//         B2DEBUG(60, "Added PXDCluster " << hitID << " from StoreArray " << aCluster.getArrayName() << " to genfit::TrackCand.");
//       }
//     }
//     // SVD SpacePoints
//     else if (detType == VXD::SensorInfoBase::SVD) {
//       RelationVector<SVDCluster> svdClusters = aHit->getRelationsTo<SVDCluster>("ALL");
//       if (svdClusters.size() == 0) {
//         B2WARNING("Could not find a relation to a SVDCluster for SpacePoint " << aHit->getArrayIndex() << " from StoreArray " << aHit->getArrayName() << ". The Hit belonging to this SpacePoint will not be in the genfit::TrackCand.");
//         continue;
//       }
//       for (const SVDCluster & aCluster : svdClusters) {
//         int hitID = aCluster.getArrayIndex();
//         returnTrackCand.addHit(Const::SVD, hitID);
//         B2DEBUG(60, "Added SVDCluster " << hitID << " from StoreArray " << aCluster.getArrayName() << " to genfit::TrackCand.");
//       }
//     } else {
//       throw UnsupportedDetType();
//     }
//   }
//   return returnTrackCand;
// }

// more or less copy pasted from genfit::TrackCand
void SpacePointTrackCand::setPdgCode(int pdgCode)
{
  m_pdg = pdgCode;
  TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_pdg);
  m_q = part->Charge() / (3.); // charge returned in units of |e|/3 by TParticlePDG -> Charge()
}

// 'Equal To' operator for easier comparison of SpacePointTrackCands (e.g. for testing this class)
// bool operator== (SpacePointTrackCand& lhs, SpacePointTrackCand& rhs)
bool SpacePointTrackCand::operator== (const SpacePointTrackCand& rhs)
{
  const auto lhsHits = this->getHits();
  const auto rhsHits = rhs.getHits();

  // if one TrackCand has no SpacePoint, equality is not possible and further comparing is not needed
  if (lhsHits.size() == 0 || rhsHits.size() == 0) {
    B2DEBUG(80, "At least one of the SpacePointTrackCands does not contain any SpacePoints");
    return false;
  }

  // compare number of SpacePoints in TrackCandidate, return false if not equal
  if (lhsHits.size() != rhsHits.size()) {
    B2DEBUG(80, "Numbers of SpacePoints in SpacePointTrackCands do not match");
    return false;
  }

  // compare pointers to SpacePoint, if one is not equal, return false
  for (unsigned int iSP = 0; iSP < lhsHits.size(); ++iSP) {
    if (lhsHits[iSP] != rhsHits[iSP]) {
      B2DEBUG(80, "SpacePoints " << iSP << " do not match");
      return false;
    }
  }
  return true;
}

// genfit::TrackCand prints to stdout, as does the Print method from ROOT TVectorD (which is invoked here).
// I build a stringstrem, which I then hand over B2DEBUG
// there is a somewhat nasty hack to intercept the output to the stdout by redirecting the stdout to a buffer, which can then be put into a stringstream. This is however platform-dependent and not very C++ like
void SpacePointTrackCand::print(int debuglevel, const Option_t* option) const
{
  stringstream output;
  output << " ======= SpacePointTrackCand::print() ======= \n";
  output << "This track candidate has " << m_trackSpacePoints.size() << " SpacePoints\n";
  output << "mcTrackId: " << m_MCTrackID << "\n";
  output << "seed values for 6D state : \n";
  B2DEBUG(debuglevel, output.str());

  // reset the output stream
  output.str(std::string(""));
  output.clear();

  // this is printed to stdout (using ROOT::Print() )
  m_state6D.Print(option);

  output << "estimates of Track Candidate: \n";
  output << "q = " << m_q << "\n";
  output << "pdgCode = " << m_pdg << "\n";

  for (unsigned int i = 0; i < m_trackSpacePoints.size(); ++i) {

    const SpacePoint* spacePoint = m_trackSpacePoints[i];
    // COULDDO: implement a print method for SpacePoints
    output << "SpacePoint " << i << " has Index " << spacePoint->getArrayIndex() << " in StoreArray " << spacePoint->getArrayName() << "\n";
  }
  B2DEBUG(debuglevel, output.str());
}
