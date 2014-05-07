/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/ExporterHitInfo.h"
#include "tracking/vxdCaTracking/ExporterTcInfo.h"
#include "tracking/vxdCaTracking/ExporterEventInfo.h"
#include "tracking/vxdCaTracking/HitExporter.h"
#include "tracking/vxdCaTracking/ThreeHitFilters.h"
#include "tracking/vxdCaTracking/LittleHelper.h"

// #include <TMathBase.h>
#include <math.h>
#include <fstream>
// #include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <TVector3.h>
// #include <boost/lexical_cast.hpp>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <tracking/dataobjects/FullSecID.h>

#include "tracking/spacePointCreation/SpacePoint.h"
#include "tracking/trackFindingVXD/sectorMapTools/SectorTools.h"
#include "tracking/vxdCaTracking/PassData.h"

// #include <list>
// #include <iostream>

using namespace std;
using namespace Belle2;
using boost::format;



HitExporter::~HitExporter()
{
  for (EventMapEntry anEvent : m_storedOutput) {
    delete anEvent.second;
  }
  m_storedOutput.clear();
}



void HitExporter::storeSensorInfo(const VXD::SensorInfoBase& aSensorInfo)
{
  /** Info dumping:
    * Radiation length:
    *
    * The radiation length is the mean distanc eover which a high-energy electron loses all but 1/e of its energy by bremsstrahlung.
    * For silicon, the radiation length is 21.82g/cm² (9.36cm)
    * The density of silicon is 2,336 g/cm³
  **/
  double radLengthX0 = 21.82; // g/cm ⁻²
  double density = 2.336; // g/cm³
  double redRadLengthX0 = radLengthX0 / density; // cm
  TVector3 local(0, 0, 0);
  double thickness = aSensorInfo.getThickness();
//  double thicknessInRadiationLength = thickness/radLengthX0;
//  double thicknessInRadiationLengthNoUnits = thickness*density/radLengthX0;
  TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local); // center of the sensor in global coords
  double radius = globalSensorPos.Perp();
  B2DEBUG(1, "StoreSensorinfo: radius " << radius << ", thickness " << thickness << ", redRadLengthX0 " << redRadLengthX0)
  string formattedSensorInfo = (boost::format("%1% %2% %3%") % radius % (thickness / redRadLengthX0) % (thickness * density)).str();
  m_geometry.push_back(formattedSensorInfo);
}



void HitExporter::prepareEvent(int n)
{
  m_eventNumber = n;
  ExporterEventInfo* pEventInfo = new ExporterEventInfo(n);
  m_storedOutput.insert(make_pair(n, pEventInfo));
  m_thisEvent = pEventInfo;
  B2DEBUG(5, "HitExporter::prepareEvent: address of m_thisEvent is pointing at: " << m_thisEvent << " map containing events has " << m_storedOutput.size() << "entries and event " << m_eventNumber << " has an event pointing at " << m_storedOutput.find(m_eventNumber)->second)
}



int HitExporter::getCurrentEventNumber()
{
  return m_thisEvent->getEventNumber();
}



int HitExporter::getNumberOfHits()
{
  std::vector<ExporterHitInfo>* hits = NULL;
  B2DEBUG(10, "HitExporter::getNumberOfHits, before getting hits, vector points at " << hits)
//  m_thisEvent->getHits(hits);
  hits = m_thisEvent->getHits();
  B2DEBUG(10, "HitExporter::getNumberOfHits, after getting hits, vector points at " << hits)
  return hits->size();
}



int HitExporter::getNumberOfPXDTrueHits()
{
  return m_thisEvent->getNPXDTrueHits();
}



int HitExporter::getNumberOfSVDTrueHits()
{
  return m_thisEvent->getNSVDTrueHits();
}



std::string HitExporter::storePXDTrueHit(VXD::GeoCache& geometry, const PXDTrueHit* aHit, int storeArrayID, bool doSmear, int isPrimaryBackgroundOrGhost, int particleID, int pdg)
{
  if (isPrimaryBackgroundOrGhost != 0 and isPrimaryBackgroundOrGhost != 1)  {
    isPrimaryBackgroundOrGhost = -1;
  }
  VxdID aVxdID = aHit->getSensorID();
  int aLayerID = aVxdID.getLayerNumber();
  B2DEBUG(10, "within HitExporter::storePXDTrueHit. Hit " << storeArrayID << " with particleID " << particleID << " and pdg " << pdg << " isPrimaryBackgroundOrGhost: " << isPrimaryBackgroundOrGhost << " has been found in sensorID " << aVxdID << " at Layer " << aLayerID)
//  const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
  return storeTrueHit(aSensorInfo, aHit->getU(), aHit->getV(), 0, storeArrayID, isPrimaryBackgroundOrGhost, aLayerID, particleID, pdg, doSmear);
}



std::string HitExporter::storeSVDTrueHit(VXD::GeoCache& geometry, const SVDTrueHit* aHit, int storeArrayID, bool doSmear, int isPrimaryBackgroundOrGhost, int particleID, int pdg)
{
  if (isPrimaryBackgroundOrGhost != 0 and isPrimaryBackgroundOrGhost != 1)  {
    isPrimaryBackgroundOrGhost = -1;
  }
  VxdID aVxdID = aHit->getSensorID();
  int aLayerID = aVxdID.getLayerNumber();
  B2DEBUG(10, "within HitExporter::storeSVDDTrueHit. Hit " << storeArrayID << " with particleID " << particleID << " and pdg " << pdg << " isPrimaryBackgroundOrGhost: " << isPrimaryBackgroundOrGhost << " has been found in sensorID " << aVxdID << " at Layer " << aLayerID)
//  const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
  return storeTrueHit(aSensorInfo, aHit->getU(), aHit->getV(), 1, storeArrayID, isPrimaryBackgroundOrGhost, aLayerID, particleID, pdg, doSmear);
}



std::string HitExporter::storeTrueHit(VXD::SensorInfoBase aSensorInfo, double u, double v, int type, int storeArrayID, int isPrimaryBackgroundOrGhost, int aLayerID, int particleID, int pdg, bool doSmear)
{
  string result = ""; // carries a message if something went wrong
  /// smear!!!
  LittleHelper smearBox = LittleHelper();
  double sigmaU = aSensorInfo.getUPitch(v) / sqrt(12.); // this is NOT a typo!
  double sigmaV = aSensorInfo.getVPitch(v) / sqrt(12.);
  if (doSmear == true) {
    B2DEBUG(100, " u/v before smearing: " << u << "/" << v)
    u = smearBox.smearValueGauss(-1000., +1000., u, sigmaU);
    v = smearBox.smearValueGauss(-1000., +1000., v, sigmaV);
    B2DEBUG(100, " u/v after smearing: " << u << "/" << v)
  }
  TVector3 hitLocal(u, v, 0.);
  TVector3 hitGlobal = aSensorInfo.pointToGlobal(hitLocal);
  TVector3 covValues(sigmaU * sigmaU, 0., sigmaV * sigmaV); // there are no good estimations for that values yet. Therefore, the classic pitch/sqrt(12) is used. The hardware guys do not expect any correlation between u and v, therefore CovUV = 0
  // determining a vector representing the global direction of the u-orientation of the sensor plane:
  TVector3 leftHit(-1., 0., 0.), rightHit(1., 0., 0.), xAxis(5., 0., 0.), origin(0., 0., 0.);
  leftHit = aSensorInfo.pointToGlobal(leftHit);
  rightHit = aSensorInfo.pointToGlobal(rightHit);
  TVector3 sensorUAxis = rightHit - leftHit;
  // getting angle:
  ThreeHitFilters filter;
//  int signOfCurvature = filter.calcSign(xAxis, origin, sensorUAxis); // in this case, the value is positive, if the angle is below 180° (grad)
  double sensorAngle = sensorUAxis. Phi() - 0.5 * M_PI; // filter.fullAngle2D(xAxis, sensorUAxis); // angle in Radians (0°-180°)
//  if ( signOfCurvature < 0 ) { sensorAngle = M_PI + sensorAngle; } else if ( signOfCurvature == 0 ) { }
  if (sensorAngle > M_PI) { sensorAngle -= 2.*M_PI; } else if (sensorAngle < -M_PI) { sensorAngle += 2.*M_PI; }

  B2DEBUG(10, " -> this hit has got U/V " << hitLocal.X() << "/" << hitLocal.Y() << " and CovUU/CovVV " << covValues.X() << "/" << covValues.Z() << " and sensorAngle " << sensorAngle)

  unsigned int chosenID = 0;
  if (m_attachedPass != NULL) {   // is only attached, if we want to print the matlabOutput
    SectorTools::NormCoords normCoords = SpacePoint::convertToNormalizedCoordinates(make_pair(u, v), aSensorInfo.getID(), &aSensorInfo);
    unsigned int mySecID = SectorTools::calcSecID(m_attachedPass->secConfigU, m_attachedPass->secConfigV, normCoords);

    VxdID myVxdID = aSensorInfo.getID();
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
      stringstream configs;
      configs << "uConfig: ";
      for (auto aVal : m_attachedPass->secConfigU) { configs << aVal << " "; }
      configs << ", vConfig: ";
      for (auto aVal : m_attachedPass->secConfigV) { configs << aVal << " "; }
      B2DEBUG(10, "calculating secID, with " << configs.str() << ", normCoords: " << normCoords.first << "/" << normCoords.second << ", calculated secID " << mySecID << " at sensor " << myVxdID)
    }
    FullSecID fullSecID0 = FullSecID(myVxdID.getLayerNumber(), false, myVxdID.getID(), mySecID);
    FullSecID fullSecID1 = FullSecID(myVxdID.getLayerNumber(), true, myVxdID.getID(), mySecID);

    int takeID = -1;
    for (auto & aSector : m_attachedPass->sectorMap) {
      if (aSector.first == fullSecID0) { takeID = 0; chosenID = fullSecID0; break; }
      if (aSector.first == fullSecID1) { takeID = 1; chosenID = fullSecID1; break; }
    }

    if (takeID == -1) {
      chosenID = fullSecID0;
      stringstream resultStream;
      resultStream << "hit with pID " << particleID << " at FullsecID " << fullSecID0 << "/" << fullSecID1 << " not found in " << m_attachedPass->sectorSetup;
      result += resultStream.str();
    }
  }

  ExporterHitInfo aHitInfo(hitGlobal, covValues, aLayerID, aSensorInfo.getID(), chosenID, sensorAngle, storeArrayID, type, isPrimaryBackgroundOrGhost, particleID, pdg);
  m_thisEvent->addHit(aHitInfo);

  return result;
}



std::string HitExporter::storeGFTC(VXD::GeoCache& geometry, const genfit::TrackCand* aTC, int tcFileIndex, int tcSimIndex, std::vector<const PXDTrueHit*> pxdHits, std::vector<const SVDTrueHit*> svdHits, std::vector<int> hitIDs)
{
  StoreArray<MCParticle> mcParticles;
  TVector3 momentum = aTC->getMomSeed();
  TVector3 vertex = aTC->getPosSeed();
  double chargeD = aTC->getChargeSeed();
  int motherID = aTC->getMcTrackId(); // mcTrackID
  if (m_outputFormat == "gsi") {
    if (mcParticles[motherID]->hasStatus(MCParticle::c_PrimaryParticle) == true) { motherID = -1; }   // primary particles have -1 here...
  } else if (m_outputFormat == "simpleMatlab") {
// do nothing
  } else {
    B2FATAL("HitExporter::storeGFTC: '" << m_outputFormat << "' is not a known outputFormat!")
  }

  int pdg = aTC->getPdgCode();
  double pVal = momentum.Mag();
  double pTVal = momentum.Perp();
  double qP = chargeD / pVal;
  momentum = momentum.Unit();
  B2DEBUG(5, "storeGFTC: momentum: " << pVal << ", pt: " << pTVal << ", qP: " << qP << ", pdg: " << pdg << " and theta of vertex: " << momentum.Theta())

  ExporterTcInfo newTC(pdg, motherID);
  B2DEBUG(5, " HitExporter::storeGFTC, executing tc with FileIndex/simIndex/motherID: " << tcFileIndex << "/" << tcSimIndex << "/" << motherID << " which has got " << pxdHits.size() << "/" << svdHits.size() << " hits")

  TVector3 hitMomentum, hitPosition;

  for (const PXDTrueHit * hit : pxdHits) {
    // collecting data:
    VxdID vID = hit->getSensorID();
    VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(vID);
    int layerID = vID.getLayerNumber();
    hitMomentum = hit->getMomentum();
    hitPosition.SetXYZ(hit->getU(), hit->getV(), 0);
    hitPosition = aSensorInfo.pointToGlobal(hitPosition);
    double hitQP = chargeD / (hitMomentum.Mag());
    double radius = hitPosition.Perp();

    // converting to string:
    stringstream hitInfo;
    if (m_outputFormat == "gsi") {
      hitInfo << hitPosition.X() << " " << hitPosition.Y() << " " << hitPosition.Z() << endl;
      hitInfo << hitMomentum.X() << " " << hitMomentum.Y() << " " << hitMomentum.Z() << " " << hitQP << endl;
      hitInfo << 0 << " " << layerID - 1 << " " << tcFileIndex << " " << motherID << " " << endl;
    } else if (m_outputFormat == "simpleMatlab") {
      hitInfo << motherID << " " << hitPosition.X() << " " << hitPosition.Y() << " " << hitPosition.Z() << " " << FullSecID(vID, false, 0) << endl;
    }

    newTC.addHit(make_pair(radius, (hitInfo).str()));
  }

  for (const SVDTrueHit * hit : svdHits) {
    // collecting data:
    VxdID vID = hit->getSensorID();
    VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(vID);
    int layerID = vID.getLayerNumber();
    hitMomentum = hit->getMomentum();
    hitMomentum = aSensorInfo.vectorToGlobal(hitMomentum);
    hitPosition.SetXYZ(hit->getU(), hit->getV(), 0);
    hitPosition = aSensorInfo.pointToGlobal(hitPosition);
    double hitQP = chargeD / (hitMomentum.Mag());
    double radius = hitPosition.Perp();

    // converting to string:
    stringstream hitInfo;
    if (m_outputFormat == "gsi") {
      hitInfo << hitPosition.X() << " " << hitPosition.Y() << " " << hitPosition.Z() << endl;
      hitInfo << hitMomentum.X() << " " << hitMomentum.Y() << " " << hitMomentum.Z() << " " << hitQP << endl;
      hitInfo << 0 << " " << layerID - 1 << " " << tcFileIndex << " " << motherID << " " << endl;
    } else if (m_outputFormat == "simpleMatlab") {
      hitInfo << motherID << " " << hitPosition.X() << " " << hitPosition.Y() << " " << hitPosition.Z() << " " << FullSecID(vID, false, 0) << endl;
    } else {
      B2FATAL("HitExporter::storeGFTC: '" << m_outputFormat << "' is not a known outputFormat!")
    }
    newTC.addHit(make_pair(radius, (hitInfo).str()));
  }
  int nHits = newTC.getNHits();

  stringstream tcInfo;
  tcInfo << vertex.X() << " " << vertex.Y() << " " << vertex.Z() << " " << momentum.X() << " " << momentum.Y() << " " << momentum.Z() << " " << qP << endl;
  tcInfo << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << " " << 0 << endl;
  tcInfo << pVal << " " << pTVal << endl;
  tcInfo << nHits << " " << nHits << " "; // at this point, the Index of the first mcPoint is not known yet, therefore tcInfo is incomplete until export
  newTC.addInfo(tcInfo.str());
  newTC.addHitIDs(hitIDs);
  m_thisEvent->addTc(newTC);

  return "EVERYTHING IS FINE";
}



std::string HitExporter::exportGsi(int runNumber, float bz)
{
  string hitsFileName, mcPointsFileName, mcTracksFileName, settingsFileName, hitLabesFileName;
  ofstream hitsFileStream, settingsFileStream, mcTracksFileStream, mcPointsFileStream, hitLabesFileStream;
  B2DEBUG(1, " HitExporter::exportGsi: got " << m_storedOutput.size() << " events, exporting to files ")
  stringstream returnStringStream;
  returnStringStream  << " in run " << runNumber << " the following events had to be dropped: ";

  // doing the eventXX_hits.data & eventXX_MCPoints.data & eventXX_MCTracks.data:
  int mcPointCounter, mcTrackCounter, droppedEventsCtr = 0, goodEventsCtr = 0, goodTCsCtr = 0;
  for (EventMapEntry eventInfo : m_storedOutput) {
    mcPointCounter = 0, mcTrackCounter = 0; // counting them manually
    typedef pair <int, int> HitIDRelation;
    vector< HitIDRelation > hitIDRelations; // hitIDRelations: stores hitIDs of all hits (real=mcPoints and background) in .first, and their connected tc in .second. If they are not connected with a tc, they are linked with -1
    // importing data:
    vector<ExporterHitInfo>* hitsOfEvent = NULL;
    hitsOfEvent = eventInfo.second->getHits();
    int nHits = hitsOfEvent->size();
    vector<ExporterTcInfo>* tcsOfEvent = NULL;
    tcsOfEvent = eventInfo.second->getTcs();
    int nMCtcs = tcsOfEvent->size();
    int nMCHits = eventInfo.second->getNMCHits();
    if (nHits == 0) {
      B2WARNING("HitExporter::exportGsi: event " << eventInfo.first << " has no hits! Dropping data...")
      droppedEventsCtr++;
      returnStringStream << eventInfo.first << " ";
      continue;
    } else if (nMCHits == 0) {
      B2WARNING("HitExporter::exportGsi: event " << eventInfo.first << " has no mcHits! Dropping data...")
      droppedEventsCtr++;
      returnStringStream << eventInfo.first << " ";
      continue;
    } else if (tcsOfEvent == 0) {
      B2WARNING("HitExporter::exportGsi: event " << eventInfo.first << " has no TCs! Dropping data...")
      droppedEventsCtr++;
      returnStringStream << eventInfo.first << " ";
      continue;
    }

    // generating filenames
    hitsFileName = (boost::format("event%1%_hits.data") % (eventInfo.first - droppedEventsCtr)).str();
    mcPointsFileName = (boost::format("event%1%_MCPoints.data") % (eventInfo.first - droppedEventsCtr)).str();
    mcTracksFileName = (boost::format("event%1%_MCTracks.data") % (eventInfo.first - droppedEventsCtr)).str();
    hitLabesFileName = (boost::format("event%1%_hitLabels.data") % (eventInfo.first - droppedEventsCtr)).str();
    //opening files:
    hitsFileStream.open(hitsFileName.c_str(), std::ios_base::trunc); // trunc=overwrite app=append
    mcPointsFileStream.open(mcPointsFileName.c_str(), std::ios_base::trunc);
    mcTracksFileStream.open(mcTracksFileName.c_str(), std::ios_base::trunc);
    hitLabesFileStream.open(hitLabesFileName.c_str(), std::ios_base::trunc);

    if (hitsOfEvent == NULL) {
      B2WARNING("event " << eventInfo.first << " has no hits! Rejecting event...")
      continue;
    }
    if (tcsOfEvent == NULL) {
      B2WARNING("event " << eventInfo.first << " has no tcs! Rejecting event...")
      continue;
    }
    B2DEBUG(1, " HitExporter::exportGsi: executing event " << eventInfo.first << " and storing " << nHits << " hits, " << nMCHits << " mcPoints and " << nMCHits << " mcTracks in files" << hitsFileName << ", " << mcPointsFileName << " and " << mcTracksFileName)

    // writing first line of files:
    hitsFileStream << nHits << endl;
    mcPointsFileStream << nMCHits << endl;
    mcTracksFileStream << nMCtcs << endl;
    hitLabesFileStream << nHits << endl;

    // executing hits
    for (ExporterHitInfo & hit : (*hitsOfEvent)) {
      hitIDRelations.push_back(make_pair(hit.getHitID(), -1)); // stores the hitIDs in the order of which they were written into the file. This order should not be destroyed, .second is -1 until a TC is found
      hitsFileStream << hit.getPositionFormatted() << hit.getCovValuesFormatted() << hit.getAdditionalInfoFormatted();
    } // looping over hits

    // executing tcs and mcPoints
    for (ExporterTcInfo & tc : (*tcsOfEvent)) {
      // importing data
      vector<int>* tcHitIDs = tc.getHitIDs(); // tcHitIDs stores hitIDs of mcPoints attached to tc
      vector< pair<double, string> >* hits = tc.getHits();
      std::sort(hits->begin(), hits->end()); // to assure, that the innermost hit is the first one
      int nMChits = hits->size();
      int pdg = tc.getPdgCode();
      int motherID = tc.getMotherID();

      // doing mcTracks
      mcTracksFileStream << motherID << " " << pdg << endl;
      mcTracksFileStream << tc.getInfo() << " " << mcPointCounter << endl;
      mcTracksFileStream << 0 << " " << 0 << " " << 1 << endl;

      B2DEBUG(10, "->HitExporter::exportGsi: executing tc with pdgCode/motherID " << pdg << "/" << motherID << " and adding " << nMChits << " hits to " << mcTracksFileName)

      // doing mcPoints
      for (const TcHitEntry & hit : (*hits)) {
        mcPointsFileStream << hit.second;
        ++mcPointCounter;
      }

      // doing hitLabels, atm I am using truehits, which are caused by only one tc. when changing to clusters, this method has to be rewritten suiting for the cases where more than one tc is using the same hit (or at least one of its strips)
      for (HitIDRelation & hitRelation : hitIDRelations) { /// the '&' is important since I want to add information. Without the '&' I am only writing into a local copy of hitIDRelations
        for (int tcHitID : (*tcHitIDs)) {
          if (hitRelation.first == tcHitID) { hitRelation.second = mcTrackCounter; break; }
        }
      }

      ++mcTrackCounter;
    } // looping over tcs
    goodTCsCtr += mcTrackCounter;

    // storing hitLabes, has to be done out of the tc-loop to be independent of tc-behavior (if there are no tcs, rel.second will be -1):
    for (const HitIDRelation & rel : hitIDRelations) {
      hitLabesFileStream << rel.second << " " << -1 << " " << -1 << endl;
    }

    if (mcPointCounter != nMCHits) {
      B2ERROR("HitExporter::exportGsi:: num of MC hits is wrong! is: " << nMCHits << ", should: " << mcPointCounter)
    }

    mcTracksFileStream << endl;
    mcPointsFileStream << endl;
    hitLabesFileStream << endl;
    hitsFileStream.close();
    mcTracksFileStream.close();
    mcPointsFileStream.close();
    hitLabesFileStream.close();
    ++goodEventsCtr;
  } // looping over events

  returnStringStream << endl << " there were " << goodEventsCtr << " accepted events having a total of " << goodTCsCtr << " TCs" << endl;

  //doing settings.data
  settingsFileName = "settings.data";
  int nLayers = m_geometry.size();
  B2DEBUG(5, " HitExporter::exportGsi: generating settings-file and storing " << nLayers << " layers in " << settingsFileName)
  settingsFileStream.open(settingsFileName.c_str(), std::ios_base::trunc);
  settingsFileStream << nLayers << endl << bz << endl; // TODO bz-Value should be able to be read from up-to-date source in case of changing values.
  for (const string & layer : m_geometry) {
    settingsFileStream << layer << endl;
  }
  settingsFileStream << endl;
  settingsFileStream.close();


  return returnStringStream.str();
}



std::string HitExporter::exportSimpleMatlab(int runNumber)
{
  string hitsFileName, mcPointsFileName;
  ofstream hitsFileStream, mcPointsFileStream;
  B2DEBUG(1, " HitExporter::exportSimpleMatlab: got " << m_storedOutput.size() << " events, exporting to files ")
  stringstream returnStringStream;
  returnStringStream  << " in run " << runNumber << " the following events had to be dropped: ";

  // doing the eventXX_hits.data & eventXX_MCPoints.data & eventXX_MCTracks.data:
  int mcPointCounter, mcTrackCounter, droppedEventsCtr = 0, goodEventsCtr = 0, goodTCsCtr = 0;
  for (EventMapEntry eventInfo : m_storedOutput) {
    mcPointCounter = 0, mcTrackCounter = 0; // counting them manually
    typedef pair <int, int> HitIDRelation;
    // importing data:
    vector<ExporterHitInfo>* hitsOfEvent = NULL;
    hitsOfEvent = eventInfo.second->getHits();
    int nHits = hitsOfEvent->size();
    vector<ExporterTcInfo>* tcsOfEvent = NULL;
    tcsOfEvent = eventInfo.second->getTcs();
    int nMCHits = eventInfo.second->getNMCHits();
    if (nHits == 0) {
      B2WARNING("HitExporter::exportSimpleMatlab: event " << eventInfo.first << " has no hits! Dropping data...")
      droppedEventsCtr++;
      returnStringStream << eventInfo.first << " ";
      continue;
    } else if (nMCHits == 0) {
      B2WARNING("HitExporter::exportSimpleMatlab: event " << eventInfo.first << " has no mcHits! Dropping data...")
      droppedEventsCtr++;
      returnStringStream << eventInfo.first << " ";
      continue;
    } else if (tcsOfEvent == 0) {
      B2WARNING("HitExporter::exportSimpleMatlab: event " << eventInfo.first << " has no TCs! Dropping data...")
      droppedEventsCtr++;
      returnStringStream << eventInfo.first << " ";
      continue;
    }

    // generating filenames
    hitsFileName = (boost::format("event%1%_hits.data") % (eventInfo.first - droppedEventsCtr)).str();
    mcPointsFileName = (boost::format("event%1%_MCPoints.data") % (eventInfo.first - droppedEventsCtr)).str();
    if (m_attachedPass != NULL) {
      hitsFileName = (boost::format("%1%_event%2%_hits.data") % m_attachedPass->sectorSetup % (eventInfo.first - droppedEventsCtr)).str();
      mcPointsFileName = (boost::format("%1%_event%2%_MCPoints.data") % m_attachedPass->sectorSetup % (eventInfo.first - droppedEventsCtr)).str();
    }
    //opening files:
    hitsFileStream.open(hitsFileName.c_str(), std::ios_base::trunc); // trunc=overwrite app=append
    mcPointsFileStream.open(mcPointsFileName.c_str(), std::ios_base::trunc);

    if (hitsOfEvent == NULL) {
      B2WARNING("event " << eventInfo.first << " has no hits! Rejecting event...")
      continue;
    }
    if (tcsOfEvent == NULL) {
      B2WARNING("event " << eventInfo.first << " has no tcs! Rejecting event...")
      continue;
    }
    B2DEBUG(1, " HitExporter::exportSimpleMatlab: executing event " << eventInfo.first << " and storing " << nHits << " hits, " << nMCHits << " mcPoints in files" << hitsFileName << ", " << mcPointsFileName)

    // writing first line of files:
    hitsFileStream << nHits << endl;
    mcPointsFileStream << nMCHits << endl;

    // executing hits
    for (ExporterHitInfo & hit : (*hitsOfEvent)) {
      hitsFileStream << hit.getSimpleHitFormatted();
    } // looping over hits

    // executing tcs and mcPoints
    for (ExporterTcInfo & tc : (*tcsOfEvent)) {
      // importing data
      vector< pair<double, string> >* hits = tc.getHits();
      std::sort(hits->begin(), hits->end()); // to assure, that the innermost hit is the first one
      int nMChits = hits->size();
      int pdg = tc.getPdgCode();
      int motherID = tc.getMotherID();

      B2DEBUG(10, "->HitExporter::exportSimpleMatlab: executing tc with pdgCode/motherID " << pdg << "/" << motherID << " and adding " << nMChits << " hits.")

      // doing mcPoints
      for (const TcHitEntry & hit : (*hits)) {
        mcPointsFileStream << hit.second;
        ++mcPointCounter;
      }

      ++mcTrackCounter;
    } // looping over tcs
    goodTCsCtr += mcTrackCounter;

    if (mcPointCounter != nMCHits) {
      B2ERROR("HitExporter::exportSimpleMatlab: num of MC hits is wrong! is: " << nMCHits << ", should: " << mcPointCounter)
    }

    mcPointsFileStream << endl;
    hitsFileStream.close();

    mcPointsFileStream.close();
    ++goodEventsCtr;
  } // looping over events

  if (m_attachedPass != NULL) {
    returnStringStream << endl << " the exporter with attached pass " << m_attachedPass->sectorSetup << " has been executed.";
  }
  returnStringStream << endl << " there were " << goodEventsCtr << " accepted events having a total of " << goodTCsCtr << " TCs" << endl;


  return returnStringStream.str();
}