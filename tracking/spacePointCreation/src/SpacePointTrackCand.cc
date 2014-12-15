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
  m_q(0),
  m_iTrackStub(-1)
{

}

SpacePointTrackCand::SpacePointTrackCand(const std::vector<const Belle2::SpacePoint*>& spacePoints, int pdgCode, double charge, int mcTrackID) :
  m_state6D(6),
  m_cov6D(6),
  m_iTrackStub(-1)
{
  m_pdg = pdgCode;
  m_q = charge;
  m_MCTrackID = mcTrackID;

  for (const SpacePoint * spacePoint : spacePoints) {
    m_trackSpacePoints.push_back(spacePoint);
    m_trackSpacePointIndices.push_back(spacePoint->getArrayIndex());
  }
}

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

// get SpacePoints in range
const std::vector<const Belle2::SpacePoint*> SpacePointTrackCand::getHitsInRange(int firstInd, int lastInd) const
{
  std::vector<const SpacePoint*> spacePoints;
  for (int iSP = firstInd; iSP <= lastInd; ++iSP) {
    spacePoints.push_back(m_trackSpacePoints[iSP]);
  }
  return spacePoints;
}

// get Sorting Parameters in range
const std::vector<double> SpacePointTrackCand::getSortingParametersInRange(int firstIndex, int lastIndex) const
{
  std::vector<double> sortingParams;
  for (int iSP = firstIndex; iSP <= lastIndex; ++iSP) {
    sortingParams.push_back(m_sortingParameters[iSP]);
  }
  return sortingParams;
}

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

// set sorting parameters
void SpacePointTrackCand::setSortingParameters(const std::vector<double>& sortParams)
{
  for (auto aValue : sortParams) { m_sortingParameters.push_back(aValue); }
}

// genfit::TrackCand prints to stdout, as does the Print method from ROOT TVectorD (which is invoked here).
// I build a stringstrem, which I then hand over B2DEBUG
// there is a somewhat nasty hack to intercept the output to the stdout by redirecting the stdout to a buffer, which can then be put into a stringstream. This is however platform-dependent and not very C++ like and therefore not done here
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

  // Only print this if the debuglevel is high enough
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, debuglevel, PACKAGENAME())) { m_state6D.Print(option); }

  output << "estimates of Track Candidate: \n";
  output << "q = " << m_q << "\n";
  output << "pdgCode = " << m_pdg << "\n";

  for (unsigned int i = 0; i < m_trackSpacePoints.size(); ++i) {

    const SpacePoint* spacePoint = m_trackSpacePoints[i];
    // COULDDO: implement a print method for SpacePoints
    output << "SpacePoint " << i << " has Index " << spacePoint->getArrayIndex() << " in StoreArray " << spacePoint->getArrayName() << ". Sorting Parameter: " << m_sortingParameters[i] << "\n";
  }
  B2DEBUG(debuglevel, output.str());
}
