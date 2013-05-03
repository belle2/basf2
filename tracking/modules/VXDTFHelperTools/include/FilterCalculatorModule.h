/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FilterCalculatorModule_H_
#define FilterCalculatorModule_H_

#include <framework/core/Module.h>
#include "tracking/vxdCaTracking/TwoHitFilters.h"
#include "tracking/vxdCaTracking/ThreeHitFilters.h"
#include "tracking/vxdCaTracking/FourHitFilters.h"
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <framework/gearbox/Const.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility> // pair
#include <TVector3.h>
#include <boost/format.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <boost/foreach.hpp>

namespace Belle2 {

  class VXDHit; /**< internal datastore for Hits */
  class VXDTrack; /**< internal datastore for tracks */
  class Sector;   /**< sector is a subunit of a sensor, please check low momentum trackfinder for more details (VXDTF) */
  class FMSectorFriends;  /**< info about compatible sectors to main sector */
  class HitFilter;  /**< internal datastore for hitinformation */


  /** internal datastore for Hits */
  class VXDHit {
  public:
    /** Constructor of class VXDHit*/
    VXDHit(short int hitType, std::string fullSecID, int aUniID, TVector3 hit, TVector3 mom, int pdg):
      m_type(hitType),
      m_secID(fullSecID),
      m_uniID(aUniID),
      m_timeStamp(0),
      m_pxdHit(NULL),
      m_svdHit(NULL),
      m_pdg(pdg) { checkDirectionOfFlight(hit, mom); }

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

    /** returns pointer of trueHit, only valid if this the VXDHit is a svdHit */
    const SVDTrueHit* getSVDHit() const { return m_svdHit; }

    /** returns pointer of trueHit, only valid if this the VXDHit is a pxdHit */
    const PXDTrueHit* getPXDHit() const { return m_pxdHit; }

    /** returns global hit position of current hit */
    TVector3 getHitPosition() { return m_hitPos; } // global coordinates

    /** returns secID of sector containing this hit */
    std::string getSectorID() { return m_secID; }

    /** returns uniID of sensor containing this hit */
    int getUniID() { return m_uniID; }

    /** setter for svdHit */
    void setSVDHit(const SVDTrueHit* aHit) {
      if (m_type == Const::SVD) {
        m_svdHit = aHit;
        m_timeStamp = aHit->getGlobalTime();
//        std::cout << " global timestamp: " << m_timeStamp << std::endl;
      } else { std::cout << "Class VXDHit of FilterCalculatorModule: input type != hitType(SVD)!" << std::endl; }
    }

    /** setter for pxdHit */
    void setPXDHit(const PXDTrueHit* aHit) {
      if (m_type == Const::PXD) {
        m_pxdHit = aHit;
        m_timeStamp = aHit->getGlobalTime();
//        std::cout << " global timestamp: " << m_timeStamp << std::endl;
      } else { std::cout << "Class VXDHit of FilterCalculatorModule: input type != hitType(PXD)!" << std::endl; }
    }

    /** setHit to vertex*/
    void setVertex() {
      m_type = Const::IR;
      m_timeStamp = 0.;
    }

  protected:
    /** checks whether particle was flying towards the IR or away from IR when producing this hit*/
    void checkDirectionOfFlight(TVector3 hitPos, TVector3 momVec) {
      m_hitPos = hitPos;
      double hitMag = hitPos.Mag();
      double momMag = (hitPos + momVec).Mag();
      if (hitMag > momMag) { m_points2IP = true; }
      else { m_points2IP = false; }
    }

    short int m_type;  /**< Const::PXD for PXD, Const::SVD for SVD, Const::IR for Primary vertex */
    bool m_points2IP;  /**< true, if momentum vector of current hit points to the interaction point (particle is flying inwards at that moment), false if not*/
    std::string m_secID; /**< ID of sector containing this hit */
    int m_uniID; /**< ID of sensor containing this hit */
    float m_timeStamp; /**< timestamp of hit (real info, needed for sorting) */
    const PXDTrueHit* m_pxdHit; /**< pointer to pxdHit (only set if it is a pxdHit) */
    const SVDTrueHit* m_svdHit; /**< pointer to svdHit (only set if it is a svdHit) */
    int m_pdg; /**< pdgCode of particle causing hit */
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

    /** returns indexNumber of current track */
    int getParticleID() { return m_index; }

    /** returns number of hits attached to curren track */
    int size() { return m_hitList.size(); }

    /** sorts hits depending on time stamp of each hit */
    void sort() { m_hitList.sort(); }

    /** reverse hitlist*/
    void reverse() { m_hitList.reverse(); }

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
    HitFilter(std::string myName):
      m_filterName(myName) {}

    /** add value to list */
    void addValue(double aValue) { m_valuePack.push_back(aValue); }

    /** returns filter type */
    std::string getFilterType() { return m_filterName; }

    /** returns list of values */
    std::list<double> getValues() { return m_valuePack; }

    /** writes values in File */
    void exportValues(std::string aFileName) {
      m_fileName = aFileName;
      m_valuePack.sort();
      m_exportValues.open(m_fileName.c_str(), std::ios_base::app); //
      m_exportValues << m_filterName << "_";
//      std::cout << m_filterName <<  " includes " << m_valuePack.size() << " entries" << std::endl;
      BOOST_FOREACH(double value, m_valuePack) {
        m_exportValues << value << "|";
      }
      m_exportValues << std::endl;
      m_exportValues.close();
    }

  protected:
    std::string m_filterName; /**< filter type (e.g. distance3D) */
    std::list<double> m_valuePack; /**< list containing all values calculated during run */
    std::ofstream m_exportValues; /**< used for file export */
    std::string m_fileName; /**< file name of exported file */
  };


  /** info about compatible sectors to main sector */
  class FMSectorFriends {
  public:
    /** Constructor of class FMSectorFriends*/
    FMSectorFriends(std::string myName):
      m_friendName(myName) {}

    /** addsValue to specified filter type */
    void addValue(std::string aFilterType, double aValue) {
      if (m_filterMap.find(aFilterType) == m_filterMap.end()) {
        HitFilter* aFilterPointer =  new HitFilter(aFilterType);
        m_filterMap.insert(std::make_pair(aFilterType, aFilterPointer));
      }
      m_filterMap.find(aFilterType)->second->addValue(aValue);
    }

    /** export filters (used at endrun) */
    void exportFilters(std::string secName, std::string setupName) {
      m_fileName = (boost::format("%1%/%1%-Sector%2%Friend%3%.txt") % setupName % secName % m_friendName).str();
      typedef std::pair<std::string, HitFilter*> mapEntry;
      BOOST_FOREACH(mapEntry thisEntry, m_filterMap) {
        thisEntry.second->exportValues(m_fileName);
        delete(thisEntry.second);
      }
    }

  protected:
    std::map<std::string, HitFilter*> m_filterMap; /**< map of filters attached to current friend */
    std::string m_friendName; /**< name of current friend */
    std::string m_fileName; /**< file name of exported file */
  };


  /** sector is a subunit of a sensor, please check low momentum trackfinder for more details (VXDTF) */
  class Sector {
  public:
    /** constructor */
    Sector(float v1, float v2, float u1v1, float u1v2, float u2v1, float u2v2, std::string myName):
      m_usageCounter(1),
      m_secName(myName),
      m_edgeO(std::make_pair(u1v1, v1)),
      m_edgeU(std::make_pair(u2v1, v1)),
      m_edgeV(std::make_pair(u1v2, v2)),
      m_edgeUV(std::make_pair(u2v2, v2)) {
      m_friendMap.clear();
    }

    /** adds new value of specified filter type for given friend */
    void addValue(std::string aFriend, std::string aFilterType, double aValue) {
      std::map<std::string, FMSectorFriends>::iterator friendPos = m_friendMap.find(aFriend);
      if (friendPos == m_friendMap.end()) {
        FMSectorFriends newFriend(aFriend);
        m_friendMap.insert(std::make_pair(aFriend, newFriend));
      }
      m_friendMap.find(aFriend)->second.addValue(aFilterType, aValue);
    }

    /** exports Friends including info to file */
    void exportFriends(std::string setupName) {
      typedef std::pair<std::string, FMSectorFriends> mapEntry;
      BOOST_FOREACH(mapEntry thisEntry, m_friendMap) {
        thisEntry.second.exportFilters(m_secName, setupName);
      }
    }

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

  protected:
    int m_usageCounter; /**< counts number of times current sector is used */
    std::string m_secName; /**< name of sector  */
    std::pair<float, float> m_edgeO; /**< local coordinates of edge defined as origin within this module */
    std::pair<float, float> m_edgeU; /**< local coordinates of edge lying in U-direction compared to edgeO */
    std::pair<float, float> m_edgeV; /**< local coordinates of edge lying in V-direction compared to edgeO */
    std::pair<float, float> m_edgeUV; /**< local coordinates of diagonally lied edge compared to edgeO */
    std::map<std::string, FMSectorFriends> m_friendMap; /**< map of friends attached to thisSector */
  };



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

  protected:
    MapOfSectors m_sectorMap; /**< sectormap contains full info about sectors (will always be calculated) */
    std::vector <MapOfSectors*>  m_sectorMaps; /**< vector contains sectormap for each range of transverse momentum chosen by m_pTcuts */
    std::vector <double> m_PARAMpTcuts; /**< minimal number of entries is 1. first entry defines lower threshold for pT in GeV/c. Each further value defines a momentum range for a new sectorMap */


    int m_PARAMtracksPerEvent; /**< contains number of tracks per event */
    int m_eventCounter; /**< knows current event number */
    int m_PARAMdetectorType; /**< sets detector type -1 for VXD, Const::PXD for PXD and Const::SVD for SVD */
    int m_longTrackCounter; /**< counts number of tracks having more than 30 hits */
    int m_longTrackletCounter; /**< counts tracklets having more than the theoretically possible number of hits (should therefore always be 0 )*/
    int m_badHitsCounter; /**< counts number of times, when when two hits with same direction of flight lie in the same sensor */
    int m_badFilterValueCtr; /**< counts number of times, when calculation of filters produced "nan" */
    int m_numOfLayers; /**< knows number of layers used in chosen detector type */
    int m_totalHitCounter; /**< counts total number of hits occured */
    int m_PARAMminTrackletLength; /**< threshold for minimum tracklet length (without IP). tracklets having less hits than this value will not be checked */

    int m_PARAMhighestAllowedLayer; /**< defines the highest layer number allowed for filtering (this way, low momentum tracks can be checked neglecting the highest layer) */

    double m_PARAMuniSigma; /**< standard value is 1/sqrt(12). Change this value for sharper or more diffuse hits (coupled with 'smearHits */
    bool m_PARAMsmearHits; /**< set true if you want to smear hits */
    bool m_PARAMnoCurler; /**< set true if you want to exclude curling parts of tracks */
    bool m_PARAMuseEvtgen; /**< set true if evtGen is used for filtergeneration, set false for pGun  */
    double m_PARAMmaxXYvertexDistance; /**< allows to abort particles having their production vertex too far away from the origin (XY-plane) */
    double m_PARAMmaxZvertexDistance; /**< allows to abort particles having their production vertex too far away from the origin (z-dist) */

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
    bool m_PARAMlogHelixFit; /**< set 'true' if you want to log delta ((helix-circle-segment-angle) / deltaZ) */
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
    bool m_PARAMlogHelixHighOccupancyFit; /**< set 'true' if you want to log delta ((helix-circle-segment-angle) / deltaZ)  - high occupancy mode */
    bool m_PARAMlogSlopeHighOccupancyRZ; /**< set 'true' to log slope in r-z-plane for line of 2 trackHits  - high occupancy mode */
    bool m_PARAMlogTRadiusHighOccupancytoIPDistance; /**< set 'true' to log the difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP  - high occupancy mode */
    bool m_PARAMlogDeltaDistCircleCenterHighOccupancy; /**< set 'true' to compare the distance of the calculated centers of track circles  - high occupancy mode */

    bool m_PARAMexportSectorCoords; /**< set true if you want to export coordinates of the sectors too */

    std::string m_PARAMsectorSetupFileName; /**< enables personal sector setups (can be loaded by the vxd track finder) */
    std::vector<std::string> m_PARAMsecMapNames; /**< enables personal sector setups (can be loaded by the vxd track finder), import one for each chosen momentum range */

    TwoHitFilters m_twoHitFilterBox; /**< includes all filters using 2 hits */
    ThreeHitFilters m_threeHitFilterBox; /**< includes all filters using 3 hits */
    FourHitFilters m_fourHitFilterBox; /**< includes all filters using 4 hits */

//    float m_percentageOfFMSectorFriends;
    double m_totalLocalCoordValue; /**< for testing purposes, adds value of each local coordinate occured in the process */
    double m_totalGlobalCoordValue; /**< for testing purposes, adds value of each global coordinate occured in the process */
    std::vector<double> m_PARAMsectorConfigU; /**< allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double> m_PARAMsectorConfigV; /**< allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<int> m_trackletLengthCounter; /**< counts the number of tracklets of each possible tracklet-length occured during process */
    std::vector<int> m_trackLengthCounter; /**< counts the number of tracklets of each possible track-length occured during process */

    std::vector<int> m_trackletMomentumCounter;  /**< counts the number of tracklets for each sectorSetup */
  private:
  };
}
#endif /* FilterCalculatorModule_H_ */
