/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include "tracking/vxdCaTracking/TwoHitFilters.h"
#include "tracking/vxdCaTracking/ThreeHitFilters.h"
#include "tracking/vxdCaTracking/FourHitFilters.h"
#include "tracking/dataobjects/FilterID.h"
#include "tracking/dataobjects/FullSecID.h"
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/VXDTFRawSecMap.h> // needed for rootExport

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility> // pair

#include <TVector3.h>
#include <TTree.h>
#include <TFile.h>

#include <boost/format.hpp>
#include <boost/math/special_functions/sign.hpp>


namespace Belle2 {


//   class VXDHit; /**< internal datastore for Hits */
  class VXDTrack; /**< internal datastore for tracks */
  class Sector;   /**< sector is a subunit of a sensor, please check low momentum trackfinder for more details (VXDTF) */
  class FMSectorFriends;  /**< info about compatible sectors to main sector */
  class HitFilter;  /**< internal datastore for hitinformation */


  /** internal datastore for Hits */
  class VXDHit {
  public:
    /** Constructor of class VXDHit*/
    VXDHit(short int hitType, std::string fullSecID, int aUniID, TVector3 hit, TVector3 mom, int pdg, TVector3 origin, float timeVal):
      m_type(hitType),
      m_secID(fullSecID),
      m_uniID(aUniID),
      m_pdg(pdg),
      m_timeStamp(timeVal)// value set by member setTrueHit(...)
//       m_pxdHit(NULL),
//       m_svdHit(NULL),
    { checkDirectionOfFlight(origin, hit, mom); }

    /** Operator '<' overloaded using hit-time-information for comparison */
    bool operator<(const VXDHit& b) const { return this->getTimeStamp() < b.getTimeStamp(); }
    /** Operator '==' overloaded using hit-time-information for comparison */
    bool operator==(const VXDHit& b) const { return this->getTimeStamp() == b.getTimeStamp(); }
    /** Operator '>' overloaded using hit-time-information for comparison */
    bool operator>(const VXDHit& b) const { return this->getTimeStamp() > b.getTimeStamp(); }

    /** returns type of VXDHit */
    short int getType() { return m_type; }

    /** returns pdg-code of particle causing this hit */
    int getPDG() { return m_pdg; }

    /** returns direction of particle movement. true for inward movement, false for outward movement */
    bool getParticleMovement() { return m_points2IP; }  // true for inward movement, false for outward movement

    /** returns timeStamp of current hit */
    float getTimeStamp() const { return m_timeStamp; }

//     /** returns pointer of trueHit, only valid if this the VXDHit is a svdHit */
//     const SVDTrueHit* getSVDHit() const { return m_svdHit; }

//     /** returns pointer of trueHit, only valid if this the VXDHit is a pxdHit */
//     const PXDTrueHit* getPXDHit() const { return m_pxdHit; }

    /** returns global hit position of current hit */
    TVector3 getHitPosition() { return m_hitPos; } // global coordinates

    /** returns secID of sector containing this hit */
    std::string getSectorID() { return m_secID; }

    /** returns uniID of sensor containing this hit */
    int getUniID() { return m_uniID; }

//    template<class aTmpl>
//     void setTrueHit(const aTmpl* aHit) { // TODO why can't I use this? (it complains about being the wrong type...)
//      m_timeStamp = aHit->getGlobalTime();
//      m_trueHit = aHit;
//     } /**< setter for the TrueHit */


    /** setHit to vertex*/
    void setVertex() {
      m_type = Const::IR;
      m_timeStamp = 0.;
    }

  protected:
    /** checks whether particle was flying towards the IR/origin or away from IR/origin when producing this hit*/
    void checkDirectionOfFlight(TVector3 origin, TVector3 hitPos, TVector3 momVec) {
      m_hitPos = hitPos;
      TVector3 vectororiginToHit = hitPos - origin;
//       origin -= hitPos; // vector hit <-> origin
//       double hitMag = origin.Mag();
      double momMag = (vectororiginToHit + momVec).Mag();
//       if (hitMag > momMag) { m_points2IP = true; }
      if (momMag < vectororiginToHit.Mag()) { m_points2IP = true; }
      else { m_points2IP = false; }
    }

    short int m_type;  /**< Const::PXD for PXD, Const::SVD for SVD, Const::IR for Primary vertex, Const::invalidDetector for Telescope */
    bool m_points2IP;  /**< true, if momentum vector of current hit points to the interaction point (particle is flying inwards at that moment), false if not*/
    std::string m_secID; /**< ID of sector containing this hit */
    int m_uniID; /**< ID of sensor containing this hit */
//     template<class TemplateTrueHit>
//     const TemplateTrueHit* m_trueHit; /**< pointer to trueHit */
//     const PXDTrueHit* m_pxdHit; /**< pointer to pxdHit (only set if it is a pxdHit) */
//     const SVDTrueHit* m_svdHit; /**< pointer to svdHit (only set if it is a svdHit) */
    int m_pdg; /**< pdgCode of particle causing hit */
    float m_timeStamp; /**< timestamp of hit (real info, needed for sorting) */
    TVector3 m_hitPos; /**< global hit position */
  };


  /** internal datastore for tracks */
  class VXDTrack {
  public:
    typedef std::map<std::string, Sector> MapOfSectors; /**< stores whole sectorMap used for storing cutoffs */

    /** Constructor of class VXDTrack, when having only the particleNumber*/
    VXDTrack(int particleNumber):
      m_index(particleNumber),
      m_pT(0.),
      m_secMap(NULL) {}

    /** Constructor of class VXDTrack*/
    VXDTrack(int particleNumber, double pT, MapOfSectors* secMapPtr):
      m_index(particleNumber),
      m_pT(pT),
      m_secMap(secMapPtr) {}

    /** adds hit to Track */
    void addHit(VXDHit hit) { m_hitList.push_back(hit); }

    /** returns hits of track */
    const std::list<VXDHit> getTrack() { return m_hitList; }

    /** returns pointer to last hit added, is NULL if no hit added so far */
    const VXDHit* getLastHit() {
      int hitPos = m_hitList.size();
      if (hitPos != 0) {
        std::list<VXDHit>::iterator thisPos = m_hitList.end(); --thisPos;
        return &(*thisPos);
      }
      return NULL;
    }

    /** returns indexNumber of current track */
    int getParticleID() { return m_index; }

    /** returns number of hits attached to curren track */
    int size() { return m_hitList.size(); }

    /** sorts hits depending on time stamp of each hit */
    void sort() { m_hitList.sort(); }

    /** reverse hitlist*/
    void reverse() { m_hitList.reverse(); }

    /** reset hitlist, should be done only, if there was a main problem with the hits included! */
    void resetHitList(std::list<VXDHit>& newList) { m_hitList = newList; }

    /** set Pt of track */
    void setPt(double mom) { m_pT = mom; }

    /** get Pt of track */
    double getPt() { return m_pT; }

    /** get pointer to sectorMap relevant for track */
    MapOfSectors* getSecMap() { return m_secMap; }

  protected:
    std::list<VXDHit> m_hitList; /**< contains hits attached to track */
    int m_index; /**< stores index number of track */
    double m_pT; /**< stores transverse momentum value for track classification */
    MapOfSectors* m_secMap; /**< pointer to sectorMap relevant for track */
  };


  /** internal datastore for hitinformation */
  class HitFilter {
  public:
    /** Constructor of class HitFilter*/
    HitFilter(unsigned int myName):
      m_filterName(myName) {}

    /** add value to list */
    void addValue(double aValue) { m_valuePack.push_back(aValue); }

    /** returns filter type */
    std::string getFilterType() { return FilterID().getFilterString(m_filterName); }

    /** returns list of values */
    VXDTFRawSecMapTypedef::CutoffValues getValues() { return m_valuePack; } // returns std::list<double>

    /** writes values in text File */
    void exportValues(std::string aFileName) {
      m_fileName = aFileName;
      m_valuePack.sort();
      m_exportValues.open(m_fileName.c_str(), std::ios_base::app);
      m_exportValues << FilterID().getFilterString(m_filterName) << "_";
      for (double value : m_valuePack) {
        m_exportValues << value << "|";
      }
      m_exportValues << std::endl;
      m_exportValues.close();
    }

  protected:
    unsigned int m_filterName; /**< filter type coded as FilterID (e.g. distance3D) */
    std::list<double> m_valuePack; /**< list containing all values calculated during run */
    std::ofstream m_exportValues; /**< used for file export */
    std::string m_fileName; /**< file name of exported file */
  };


  /** info about compatible sectors to main sector */
  class FMSectorFriends {
  public:
    typedef std::pair<unsigned int, HitFilter*> MapEntry; /**< Entry of sectorMap */

    /** Constructor of class FMSectorFriends*/
    FMSectorFriends(std::string myName):
      m_friendName(myName) {}

    /** addsValue to specified filter type */
    void addValue(unsigned int aFilterType, double aValue) {
      if (m_filterMap.find(aFilterType) == m_filterMap.end()) {
        HitFilter* aFilterPointer =  new HitFilter(aFilterType);
        m_filterMap.insert(std::make_pair(aFilterType, aFilterPointer));
      }
      m_filterMap.find(aFilterType)->second->addValue(aValue);
    }

    /** export filters to text file (used at endrun) */
    void exportFilters(std::string secName, std::string setupName) {
      m_fileName = (boost::format("%1%/%1%-Sector%2%Friend%3%.txt") % setupName % secName % m_friendName).str();
      for (MapEntry thisEntry : m_filterMap) {
        thisEntry.second->exportValues(m_fileName);
      }
    }

    /** export filters to root file (used at endrun) */
    VXDTFRawSecMapTypedef::FriendValues exportFiltersRoot() {
      //        std::vector< std::pair<unsigned int, std::list<double> > >
      VXDTFRawSecMapTypedef::FriendValues completeInfoOfFriend;
      for (MapEntry thisEntry : m_filterMap) {
        completeInfoOfFriend.push_back(make_pair(thisEntry.first, thisEntry.second->getValues()));
      }
      return completeInfoOfFriend;
    }

    /** clears info of all filters stored in current friend */
    void clearFilters() {
      for (MapEntry thisEntry : m_filterMap) {
        delete(thisEntry.second);
      }
    }

  protected:
    std::map<unsigned int, HitFilter*> m_filterMap; /**< map of filters attached to current friend, is coded in FilterID which can be treated as unsigned int */
    std::string m_friendName; /**< name of current friend */
    std::string m_fileName; /**< file name of exported file */
  };


  /** sector is a subunit of a sensor, please check low momentum trackfinder for more details (VXDTF) */
  class Sector {
  public:
    typedef std::pair<std::string, FMSectorFriends> MapEntry;  /**< Entry of sectorMap */
    typedef std::pair<double, double> LocalCoordinates; /**< stores u and v in local coordinates */

    /** constructor */
    Sector(std::string myName, LocalCoordinates edge0, LocalCoordinates edgeU, LocalCoordinates edgeV, LocalCoordinates edgeUV, double distance):
      m_usageCounter(1),
      m_secName(myName),
      m_edgeO(edge0),
      m_edgeU(edgeU),
      m_edgeV(edgeV),
      m_edgeUV(edgeUV),
      m_distanceToOrigin(distance) {
      m_friendMap.clear();
    }

    /** adds new value of specified filter type for given friend */
    void addValue(std::string aFriend, unsigned int aFilterType, double aValue) {
      std::map<std::string, FMSectorFriends>::iterator friendPos = m_friendMap.find(aFriend);
      if (friendPos == m_friendMap.end()) {
        FMSectorFriends newFriend(aFriend);
        m_friendMap.insert(std::make_pair(aFriend, newFriend));
      }
      m_friendMap.find(aFriend)->second.addValue(aFilterType, aValue);
    }

    /** exports Friends including info to text file */
    void exportFriends(std::string setupName) {
      for (MapEntry thisEntry : m_friendMap) {
        thisEntry.second.exportFilters(m_secName, setupName);
      }
    }

    /** clears info of all friends stored in current sector */
    void clearFriends() {
      for (MapEntry thisEntry : m_friendMap) {
        thisEntry.second.clearFilters();
      }
    }

    /** exports Friends including info to root file */
    VXDTFRawSecMapTypedef::SectorValues exportFriendsRoot() {
      VXDTFRawSecMapTypedef::SectorValues completeInfoOfSector;
      for (MapEntry thisEntry : m_friendMap) {
        completeInfoOfSector.push_back(make_pair(FullSecID(thisEntry.first).getFullSecID(), thisEntry.second.exportFiltersRoot()));
      }
      return completeInfoOfSector;
    }

    /** returns name of sector */
    std::string getName() { return m_secName; }

    /** returns name of sector */
    std::string getSectorID() { return m_secName; }

    /** increases usage counter */
    void increaseCounter() { m_usageCounter++; }

    /** decreases usage counter (needed when double hits are detected)*/
    void decreaseCounter() { m_usageCounter--; }

    /** returns edge0 (local coordinates) */
    std::pair<float, float> getEdgeO() { return m_edgeO; }

    /** returns edgeU (local coordinates) */
    std::pair<float, float> getEdgeU() { return m_edgeU; }

    /** returns edgeV (local coordinates) */
    std::pair<float, float> getEdgeV() { return m_edgeV; }

    /** returns edgeUV (local coordinates) */
    std::pair<float, float> getEdgeUV() { return m_edgeUV; }

    /** returns distance of sector to origin of current secMap-geometry */
    double getDistance2Origin() { return m_distanceToOrigin; }

  protected:
    int m_usageCounter; /**< counts number of times current sector is used */
    std::string m_secName; /**< name of sector  */
    std::pair<float, float> m_edgeO; /**< local coordinates of edge defined as origin within this module */
    std::pair<float, float> m_edgeU; /**< local coordinates of edge lying in U-direction compared to edgeO */
    std::pair<float, float> m_edgeV; /**< local coordinates of edge lying in V-direction compared to edgeO */
    std::pair<float, float> m_edgeUV; /**< local coordinates of diagonally lied edge compared to edgeO */
    std::map<std::string, FMSectorFriends> m_friendMap; /**< map of friends attached to thisSector */
    double m_distanceToOrigin; /**< stores Info of distance of this sector (using the the center of the sector) to the origin for current sector map (used by TB-version of VXDTF to determine where are the innermost and where are the outermost sectors) */
  };


  /** The FilterCalculatorModule
   *
   * this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF.
   * This information can be exported via ascii files (not recommended) and root files (default, recommended).
   *
   */
  class FilterCalculatorModule : public Module {


  public:
    typedef std::map<std::string, Sector> MapOfSectors;  /**< stores whole sectorMap used for storing cutoffs */
    typedef std::pair<std::string, Sector> SecMapEntry; /**< represents an entry of the MapOfSectors */

    FilterCalculatorModule();

    virtual ~FilterCalculatorModule();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    virtual void terminate();

    /** initialize variables to avoid nondeterministic behavior */
    void InitializeVariables() {
      m_pxdHitCounter = 0;
      m_svdHitCounter = 0;
      m_telHitCounter = 0;
      m_useTEL = false;
      m_usePXD = false;
      m_useSVD = false;
      m_eventCounter = 0;
      m_badHitsCounter = 0;
      m_badTrackletCounter = 0;
      m_totalHitCounter = 0;
      m_longTrackCounter = 0;
      m_badFilterValueCtr = 0;
      m_longTrackletCounter = 0;
      m_numOfLayers = 0;
      m_rootFilePtr = NULL;
      m_treeEventWisePtr = NULL;
      m_rootpTValuesInLayer1Ptr = NULL;
      m_rootmomValuesInLayer1Ptr = NULL;
    }

    template<class Tmpl>
    bool createSectorAndHit(Belle2::Const::EDetector detectorID, int pdg, const Tmpl* aSiTrueHitPtr, VXDTrack& newTrack, MapOfSectors* thisSecMap); /**< internal member - takes hit, calculates sector (and creates it if it is not existing yet) and creates internal VXDHit for further calculation. If return value is true, everything worked fine. If not, then hit was not created (no info about the sector-creation) */

    /** return chosen origin for secMap-creation */
    TVector3 getOrigin() { return m_origin; }
  protected:
    MapOfSectors m_sectorMap; /**< sectormap contains full info about sectors (will always be calculated) */
    std::vector <MapOfSectors*>  m_sectorMaps; /**< vector contains sectormap for each range of transverse momentum chosen by m_pTcuts */
    std::vector <double> m_PARAMpTcuts; /**< minimal number of entries is 1. first entry defines lower threshold for pT in GeV/c. Each further value defines a momentum range for a new sectorMap */


    int m_PARAMtracksPerEvent; /**< contains number of tracks per event */
    int m_eventCounter; /**< knows current event number */
//     int m_PARAMdetectorType; /**< sets detector type -1 for VXD, Const::PXD for PXD and Const::SVD for SVD */ WARNING OLD!
    std::vector<std::string>  m_PARAMdetectorType; /**< defines which detector type has to be exported. Like geometry, simply add the detector types you want to include in the track candidates. Currently supported: PXD, SVD, VXD and TEL */
    int m_longTrackCounter; /**< counts number of tracks having more than 30 hits */
    std::string m_detectorName; /**< string of accepted detector names */
    int m_longTrackletCounter; /**< counts tracklets having more than the theoretically possible number of hits (should therefore always be 0 )*/
    int m_badHitsCounter; /**< counts number of times, when when two hits with same direction of flight lie in the same sensor */
    int m_badTrackletCounter; /**< counts number of times, when Tracklet went postal */
    int m_badFilterValueCtr; /**< counts number of times, when calculation of filters produced "nan" */
    int m_numOfLayers; /**< knows number of layers used in chosen detector type */
    int m_totalHitCounter; /**< counts total number of hits occured */
    int m_PARAMminTrackletLength; /**< threshold for minimum tracklet length (without IP). tracklets having less hits than this value will not be checked */

    int m_PARAMhighestAllowedLayer; /**< defines the highest layer number allowed for filtering (this way, low momentum tracks can be checked neglecting the highest layer) */

    short m_filterCharges; /**< this value can be set to: 1: allow only particles with positive charges, 0: allow all particles, -1: allow only particles with negative charges - standard is 0 */
    double m_PARAMuniSigma; /**< standard value is 1/sqrt(12). Change this value for sharper or more diffuse hits (coupled with 'smearHits */
    bool m_PARAMsmearHits; /**< set true if you want to smear hits */
    bool m_PARAMnoCurler; /**< set true if you want to exclude curling parts of tracks */
    bool m_PARAMuseEvtgen; /**< set true if evtGen is used for filtergeneration, set false for pGun  */
    double m_PARAMmaxXYvertexDistance; /**< allows to abort particles having their production vertex too far away from the origin (XY-plane) */
    double m_PARAMmaxZvertexDistance; /**< allows to abort particles having their production vertex too far away from the origin (z-dist) */
    std::vector<double> m_PARAMsetOrigin; /**< allows to reset orign (e.g. usefull for special cases like testbeams), only valid if 3 entries are found */
    int m_PARAMtestBeam; /**< some things which are important for the real detector are a problem for testbeams, if you want to use the filterCalculator and the testBeam = false does not work, then try setting the parameter to true */
    bool m_PARAMmultiHitsAllowed; /**< if this parameter is true, the FilterCalculatorModule ignores tracks which have more than one hit on the same sensor. If false, these tracks get filtered. There will be a warning, if parameter 'testBeam' is != 0 and this parameter is true, since there curlers shouldn't be possible */
    std::vector<double> m_PARAMacceptedRegionForSensors;  /**< accepts pair of input values. first one defines minimal distance for sectors to the origin and second one defines maximum accepted distance for sectors. If anyone of these values is above 0, sectors will be sorted using their distance2Origin parameter, not their layerID */
    double m_PARAMmagneticFieldStrength; /**< strength of magnetic field in Tesla, standard is 1.5T */

    bool m_PARAMsecMapWriteToAscii; /**< if true, secMap data is stored to ascii files (standard setting is true)*/
    // rootStuff:

    bool m_PARAMsecMapWriteToRoot; /**< if true, secMap data is stored to root file with file name chosen by 'rootFileName' + 'SecMap.root' */
    bool m_PARAManalysisWriteToRoot; /**< if true, analysis data is stored to root file with file name chosen by 'rootFileName' + 'Analysis.root'*/
    std::vector<std::string> m_PARAMrootFileName; /**< only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot' = true */
    TFile* m_rootFilePtr; /**< pointer at root file used for p-value-output */
    TTree* m_treeEventWisePtr; /**< pointer at root tree used for p-value-output, will be filled eventWise */
    std::vector<double> m_rootpTValuesInLayer1; /**< used to store all pT values of tracks passing layer 1 */
    std::vector<double> m_rootmomValuesInLayer1; /**< used to store all momentum values of tracks passing layer 1 */
    std::vector<double>* m_rootpTValuesInLayer1Ptr; /**< pointer to m_rootpTValuesInLayer1 (needed for root) */
    std::vector<double>* m_rootmomValuesInLayer1Ptr; /**< pointer to m_rootmomValuesInLayer1 (needed for root) */


    bool m_PARAMtrackErrorTracks; /**< track tracks which cause strange results */
    bool m_PARAMlogDistanceXY; /**< set 'true' if you want to log distances (XY) between trackHits */
    bool m_PARAMlogDistanceZ; /**< set 'true' if you want to log distances (Z) between trackHits */
    bool m_PARAMlogDistance3D; /**< set 'true' if you want to log distances (3D) between trackHits */
    bool m_PARAMlogPt; /**< set 'true' if you want to log Pt between segments */
    bool m_PARAMlogAngles3D; /**< set 'true' if you want to log angles3D between segments */
    bool m_PARAMlogAnglesXY; /**< set 'true' if you want to log anglesXY between segments */
    bool m_PARAMlogAnglesRZ; /**< set 'true' if you want to log anglesRZ between segments */
    bool m_PARAMlogDeltaPt; /**< set 'true' if you want to log delta Pt between segments */
    bool m_PARAMlogDeltaSlopeRZ; /**< set 'true' if you want to log delta slopes in r-z-plane between segments */
    bool m_PARAMlogHelixParameterFit; /**< set 'true' if you want to log delta ((helix-circle-segment-angle) / deltaZ) */
    bool m_PARAMlogDeltaSOverZ; /**< set 'true' if you want to log Delta S (arc-length) over Z */
    bool m_PARAMlogDeltaSlopeZOverS; /**< set 'true' if you want to log Delta of the slope Z over S */
    bool m_PARAMlogNormedDistance3D; /**< set 'true' to log improved 3D distance between trackHits */
    bool m_PARAMlogSlopeRZ; /**< set 'true' to log slope in r-z-plane for line of 2 trackHits */
    bool m_PARAMlogTRadiustoIPDistance; /**< set 'true' to log the difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP */
    bool m_PARAMlogDeltaDistCircleCenter; /**< set 'true' to compare the distance of the calculated centers of track circles */
    bool m_PARAMlogPtHighOccupancy; /**< set 'true' if you want to log Pt between segments - high occupancy mode */
    bool m_PARAMlogAnglesHighOccupancy3D; /**< set 'true' if you want to log angles3D between segments  - high occupancy mode */
    bool m_PARAMlogAnglesHighOccupancyXY; /**< set 'true' if you want to log anglesXY between segments  - high occupancy mode */
    bool m_PARAMlogAnglesHighOccupancyRZ; /**< set 'true' if you want to log anglesRZ between segments  - high occupancy mode */
    bool m_PARAMlogDeltaPtHighOccupancy; /**< set 'true' if you want to log delta Pt between segments  - high occupancy mode */
    bool m_PARAMlogDeltaSlopeHighOccupancyRZ; /**< set 'true' if you want to log delta slopes in r-z-plane between segments  - high occupancy mode */
    bool m_PARAMlogHelixParameterHighOccupancyFit; /**< set 'true' if you want to log delta ((helix-circle-segment-angle) / deltaZ)  - high occupancy mode */
//     bool m_PARAMlogSlopeHighOccupancyRZ; /**< set 'true' to log slope in r-z-plane for line of 2 trackHits  - high occupancy mode */
    bool m_PARAMlogTRadiusHighOccupancytoIPDistance; /**< set 'true' to log the difference between the radius of the track circle in x-y-plan and the distance of the center of the circle tdeltaSOverZo the IP  - high occupancy mode */
    bool m_PARAMlogDeltaDistCircleCenterHighOccupancy; /**< set 'true' to compare the distance of the calculated centers of track circles  - high occupancy mode */

    // debug filters, should only be logged if they shall be used for testing!
    bool m_PARAMlogAlwaysTrue2Hit; /**< set 'true' if you want to log AlwaysTrue2Hit between segments */
    bool m_PARAMlogAlwaysFalse2Hit; /**< set 'true' if you want to log AlwaysFalse2Hit between segments */
    bool m_PARAMlogRandom2Hit; /**< set 'true' if you want to log Random2Hit between segments */
    bool m_PARAMlogAlwaysTrue3Hit; /**< set 'true' if you want to log AlwaysTrue2Hit between segments */
    bool m_PARAMlogAlwaysFalse3Hit; /**< set 'true' if you want to log AlwaysFalse2Hit between segments */
    bool m_PARAMlogRandom3Hit; /**< set 'true' if you want to log Random2Hit between segments */
    bool m_PARAMlogAlwaysTrue4Hit; /**< set 'true' if you want to log AlwaysTrue2Hit between segments */
    bool m_PARAMlogAlwaysFalse4Hit; /**< set 'true' if you want to log AlwaysFalse2Hit between segments */
    bool m_PARAMlogRandom4Hit; /**< set 'true' if you want to log Random2Hit between segments */
    bool m_PARAMlogZigZagXY; /**< WARNING ZigZagXY is not a debug filter but since it does not work using cutoffs, this Filter shall not be logged, since it only collects random data (this logging is needed to bypass a design-flaw, which discards sectorMaps which only carry Filters which do not need cutoffs) */
    bool m_PARAMlogZigZagXYWithSigmas; /**< WARNING ZigZagXYWithSigmas is not a debug filter but since it does not work using cutoffs, this Filter shall not be logged, since it only collects random data (this logging is needed to bypass a design-flaw, which discards sectorMaps which only carry Filters which do not need cutoffs) */
    bool m_PARAMlogZigZagRZ; /**< WARNING ZigZagRZ is not a debug filter but since it does not work using cutoffs, this Filter shall not be logged, since it only collects random data (this logging is needed to bypass a design-flaw, which discards sectorMaps which only carry Filters which do not need cutoffs) */

    bool m_PARAMexportSectorCoords; /**< set true if you want to export coordinates of the sectors too */

    bool m_PARAMuseOldSecCalc; /**< WARNING DEBUG - if true, old way to calc secID is used April15th-2014 */

    std::string m_PARAMsectorSetupFileName; /**< enables personal sector setups (can be loaded by the vxd track finder) */
    std::vector<std::string> m_PARAMsecMapNames; /**< enables personal sector setups (can be loaded by the vxd track finder), import one for each chosen momentum range */

    TwoHitFilters m_twoHitFilterBox; /**< includes all filters using 2 hits */
    ThreeHitFilters m_threeHitFilterBox; /**< includes all filters using 3 hits */
    FourHitFilters m_fourHitFilterBox; /**< includes all filters using 4 hits */

    std::vector<double> m_PARAMsectorConfigU; /**< allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double> m_PARAMsectorConfigV; /**< allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<int> m_trackletLengthCounter; /**< counts the number of tracklets of each possible tracklet-length occured during process */
    std::vector<int> m_trackLengthCounter; /**< counts the number of tracklets of each possible track-length occured during process */
    int m_pxdHitCounter; /**< counts total number of pxd true hits */
    int m_svdHitCounter; /**< counts total number of svd true hits */
    int m_telHitCounter; /**< counts total number of telescope true hits */
    std::vector<int> m_trackletMomentumCounter;  /**< counts the number of tracklets for each sectorSetup */
    TVector3 m_origin; /**< this point in space is assumed to be the interaction point (no matter whether the real IP lies there) */
    bool m_useTEL; /**< is set to true, if Telescope hits have to be used for tracking too */
    bool m_usePXD; /**< is set to true, if PXD hits have to be used for tracking too */
    bool m_useSVD; /**< is set to true, if SVD hits have to be used for tracking too */
  private:
  };
}
