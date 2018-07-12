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
#include <framework/gearbox/Const.h>

// SpacePointTrackCand.h
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <TDatabasePDG.h>
#include <TVectorD.h>
#include <TMatrixD.h>

// debugging
#include <sstream> // used in print method

using namespace std;
using namespace Belle2;


SpacePointTrackCand::SpacePointTrackCand(const std::vector<const Belle2::SpacePoint*>& spacePoints, int pdgCode,
                                         double charge, int mcTrackID)
{
  m_pdg = pdgCode;
  m_q = charge;
  m_MCTrackID = mcTrackID;

  double index = 0; /**< default sorting parameters */
  for (const SpacePoint* spacePoint : spacePoints) {
    m_trackSpacePoints.push_back(spacePoint);
    m_sortingParameters.push_back(index);
    index++;
  }
}

// 'Equal To' operator for easier comparison of SpacePointTrackCands (e.g. for testing this class)
// bool operator== (SpacePointTrackCand& lhs, SpacePointTrackCand& rhs)
bool SpacePointTrackCand::operator== (const SpacePointTrackCand& rhs)
{
  const auto& lhsHits = this->getHits();
  const auto& rhsHits = rhs.getHits();

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

// get SpacePoints in range
const std::vector<const Belle2::SpacePoint*> SpacePointTrackCand::getHitsInRange(int firstIndex, int lastIndex) const
{
  if (lastIndex < firstIndex) { // exchange ranges if they are in wrong order
    int tmp = firstIndex;
    firstIndex = lastIndex;
    lastIndex = tmp;
  }
  // check if the indices are in range!
  if (firstIndex < 0 || uint(lastIndex) > m_trackSpacePoints.size() || uint(firstIndex) > m_trackSpacePoints.size()
      || lastIndex < 0) {
    throw SPTCIndexOutOfBounds();
  }
  const std::vector<const SpacePoint*> spacePoints(m_trackSpacePoints.begin() + firstIndex, m_trackSpacePoints.begin() + lastIndex);
  return spacePoints;
}

// get Sorting Parameters in range
const std::vector<double> SpacePointTrackCand::getSortingParametersInRange(int firstIndex, int lastIndex) const
{
  if (lastIndex < firstIndex) { // exchange ranges if they are in wrong order
    int tmp = firstIndex;
    firstIndex = lastIndex;
    lastIndex = tmp;
  }
  // check if the indices are in range!
  if (firstIndex < 0 || uint(lastIndex) > m_sortingParameters.size() || uint(firstIndex) > m_sortingParameters.size()
      || lastIndex < 0) {
    throw SPTCIndexOutOfBounds();
  }
  const std::vector<double> sortingParams(m_sortingParameters.begin() + firstIndex, m_sortingParameters.begin() + lastIndex);
  return sortingParams;
}

// get sorted hits
const std::vector<const SpacePoint*> SpacePointTrackCand::getSortedHits() const
{
  std::vector<std::pair<const SpacePoint*, double>> sortVector;
  sortVector.reserve(m_trackSpacePoints.size());
  for (unsigned int index = 0; index < m_trackSpacePoints.size(); ++index) {
    sortVector.push_back(std::make_pair(m_trackSpacePoints.at(index), m_sortingParameters.at(index)));
  }

  std::sort(sortVector.begin(), sortVector.end(), [](const std::pair<const SpacePoint*, double>& a,
  const std::pair<const SpacePoint*, double>& b) {
    return a.second < b.second;
  });


  std::vector<const SpacePoint*> sortedSpacePoints;
  //sortedSpacePoints.reserve(m_trackSpacePoints.size());
  for (auto pair : sortVector) {
    sortedSpacePoints.push_back(pair.first);
  }

  return sortedSpacePoints;
}

// more or less copy pasted from genfit::TrackCand
void SpacePointTrackCand::setPdgCode(int pdgCode)
{
  m_pdg = pdgCode;
  TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_pdg);
  m_q = part->Charge() / (3.); // charge returned in units of |e|/3 by TParticlePDG -> Charge()
}

// set sorting parameters
void SpacePointTrackCand::setSortingParameters(const std::vector<double>& sortParams)
{
  if (sortParams.size() != m_sortingParameters.size())
    throw SPTCSortingParameterSizeInvalid();
  for (size_t iSP = 0; iSP < sortParams.size(); ++iSP) {
    m_sortingParameters.at(iSP) = sortParams.at(iSP);
  }
}

// remove a SpacePoint by index
void SpacePointTrackCand::removeSpacePoint(int indexInTrackCand)
{
  // check if the index is in bounds
  if (uint(indexInTrackCand) >= m_trackSpacePoints.size()) { throw SPTCIndexOutOfBounds(); }

  // erase the entry from vector
  m_trackSpacePoints.erase(m_trackSpacePoints.begin() + indexInTrackCand);
  m_sortingParameters.erase(m_sortingParameters.begin() + indexInTrackCand);
}

// genfit::TrackCand prints to stdout, as does the Print method from ROOT TVectorD (which is invoked here).
// I build a stringstrem, which I then hand over B2DEBUG
// there is a somewhat nasty hack to intercept the output to the stdout by redirecting the stdout to a buffer, which can then be put into a stringstream. This is however platform-dependent and not very C++ like and therefore not done here (this would be needed for having the ROOT output in the log files which are created from within a steering file)
// however the LogSystem provides a way to check if ROOT output is actually needed (check the LogLevel and the DebugLevel)
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
  output << ", QI = " << m_qualityIndicator << "\n";

  unsigned nSP = 0;
  for (const SpacePoint* spacePoint : getHits()) {
    // COULDDO: implement a print method for SpacePoints
    output << "SpacePoint " << nSP << " has Index " << spacePoint->getArrayIndex() << " in StoreArray " << spacePoint->getArrayName() <<
           "." << "\n";
    nSP++;
  }

  B2DEBUG(debuglevel, output.str());

  // reset the output stream
  output.str(std::string(""));
  output.clear();

  output << "referee properties of this SPTC:\n";
  output << "checked for SPs on same sensors: " << this->hasRefereeStatus(c_checkedSameSensors) << " -> result: " <<
         this->hasRefereeStatus(c_hitsOnSameSensor) << "\n";
  output << "checked for min distance between SPs: " << this->hasRefereeStatus(c_checkedMinDistance) << " -> result: " <<
         this->hasRefereeStatus(c_hitsLowDistance) << "\n";
  output << "checked for curling: " << this->checkedForCurling() << " -> result: " << this->isCurling() <<
         ", part of a curling SPTC: " << this->isPartOfCurlingTrack() << "\n";
  output << "direction of flight: " << m_flightDirection << ", removed SpacePoints: " << this->hasRefereeStatus(
           c_removedHits) << "\n";
  output << "omitted Clusters: " << hasRefereeStatus(c_omittedClusters) << ", single Cluster SPs: " << hasRefereeStatus(
           c_singleClustersSPs) << "\n";
  B2DEBUG(debuglevel, output.str());
}

// get referee status as string
// COULDDO: this can possibly be done with switch - case, which is probably faster
std::string SpacePointTrackCand::getRefereeStatusString(std::string delimiter) const
{
  std::string statusString;
  if (getRefereeStatus() == 0) return statusString; // return empty string if there is no status
  if (hasRefereeStatus(c_checkedByReferee)) statusString += "checkedByReferee" + delimiter;
  if (hasRefereeStatus(c_checkedClean)) statusString += "checkedClean" + delimiter;
  if (hasRefereeStatus(c_hitsOnSameSensor)) statusString += "hitsOnSameSensor" + delimiter;
  if (hasRefereeStatus(c_hitsLowDistance)) statusString += "hitsLowDistance" + delimiter;
  if (hasRefereeStatus(c_removedHits)) statusString += "removedHits" + delimiter;
  if (hasRefereeStatus(c_checkedTrueHits)) statusString += "checkedTrueHits" + delimiter;
  if (hasRefereeStatus(c_checkedSameSensors)) statusString += "checkedSameSensors" + delimiter;
  if (hasRefereeStatus(c_checkedMinDistance)) statusString += "checkedMinDistance" + delimiter;
  if (hasRefereeStatus(c_curlingTrack)) statusString += "curlingTrack" + delimiter;
  if (hasRefereeStatus(c_omittedClusters)) statusString += "omittedClusters" + delimiter;
  if (hasRefereeStatus(c_singleClustersSPs)) statusString += "singleClusterSPs" + delimiter;
  if (hasRefereeStatus(c_isActive)) statusString += "isActive" + delimiter;
  if (hasRefereeStatus(c_isReserved)) statusString += "isReserved" + delimiter;

  statusString.erase(statusString.end() - delimiter.size(),
                     statusString.end()); // remove last delimiter -> no error catching SHOULD be neccessary since the case of an empty (==0) refereeStatus is already dealt with above!
  return statusString;
}
