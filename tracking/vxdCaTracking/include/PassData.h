/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// typedefs:
#include <tracking/dataobjects/TrackFinderVXDTypedefs.h>

#include <string>
#include <TVector3.h>

#ifndef __CINT__
#include <boost/unordered_map.hpp>
#endif

#include "tracking/vxdCaTracking/VXDSector.h"
#include "tracking/vxdCaTracking/VXDTFHit.h"
#include "tracking/vxdCaTracking/VXDSegmentCell.h"
#include "tracking/vxdCaTracking/VXDTFTrackCandidate.h"

#include "tracking/vxdCaTracking/SegFinderFilters.h"
#include "tracking/vxdCaTracking/NbFinderFilters.h"
#include "tracking/vxdCaTracking/TcFourHitFilters.h"
#include "tracking/vxdCaTracking/TrackletFilters.h"

namespace Belle2 {


//   typedef std::pair<unsigned int, VXDSector*> MapOfSectorsEntry; /**< a single entry of MapOfSectors (next line) */
  typedef boost::unordered_map<unsigned int, VXDSector*> MapOfSectors; /**< stores whole sectorMap used for storing cutoffs */
  typedef std::pair<unsigned int, MapOfSectors::iterator>
  SectorNameAndPointerPair; /**< we are storing the name of the sector (which is encoded into an int) to be able to sort them! */
//   typedef std::list<SectorNameAndPointerPair> OperationSequenceOfActivatedSectors; /**< contains all active sectors, can be sorted by name (first entry) */
  typedef std::vector<VXDSector*> OperationVectorOfActivatedSectors; /**< tcontains all active sectors */
  typedef std::list<VXDSegmentCell*> ActiveSegmentsOfEvent; /**< is list since random deleting processes are needed */
  typedef std::vector<VXDSegmentCell*>
  TotalSegmentsOfEvent; /**< is vector since no entries are deleted and random access is needed  */
  typedef std::vector<VXDTFHit*> HitsOfEvent; /**< contains pointers to all hits of event */
  typedef std::vector<VXDTFTrackCandidate*> TCsOfEvent; /**< contains all track candidates of event */

  /** all the information of a pass is stored within a copy of that class. This allows grouping of that information. WARNING: at the moment, this class is still a hybrid between a struct and a class. A full set of setters and getters is missing yet and since there are so many member variables, it will probably never implemented...*/
  class PassData {

  public:

    /** Return the name of this PassData, needed to link the
    current pass to the set of Filters */
    const std::string getName() { return sectorSetup; };


    /** an exception thrown if sector not found */
    BELLE2_DEFINE_EXCEPTION(Bad_Sector, "secMap - could not find centersector!");
    /**
     * the following function retrieves a map of sectors used by VXDTF and CKFTF out of rawData given to it.
     * input is the raw sector map which is a vector of sectors.
     * output is a pair of counters, where:
     * .first is counting the total number of sectorFriends (connected inner sectors) and
     * .second is counting the total number of cutoffs stored
     */
    std::pair<int, int> importSectorMap(const VXDTFSecMapTypedef::SecMapCopy& rawSecMap,
                                        const VXDTFRawSecMapTypedef::SectorDistancesMap& distancesMap, bool useDistances);


    /** fills in standard-settings for all filters (except debug-filters like alwaysFalse2Hit) */
    void activateAllFilters();

    /** for streching the limits of the cutoff values for finetuning */
    double addExtraGain(double prefix, double cutOff, double specialTune)
    {
      if (cutOff > 0.) { return cutOff + prefix * cutOff * this->generalTune + prefix * cutOff * specialTune; }
      return cutOff - prefix * cutOff * this->generalTune - prefix * cutOff * specialTune;
    }



    /** clears all event-wise data **/
    void cleanPass()
    {
      for (VXDSector* aSector : this->sectorVector) {  // sectors stay alive, therefore only reset!
        aSector->resetSector();
      }

      MapOfSectors::iterator centerSectorKey = this->sectorMap.find(this->centerSector);
      if (centerSectorKey == this->sectorMap.end()) {
        // cleanPass() is applied on an empty pass the centerSector cannot be found, so the exception makes no sense
        if (this->sectorMap.size() != 0) throw Bad_Sector();
      } else {
        centerSectorKey->second->resetSector(); // doing the same for the virtual sector which is not in the operation sequence
      }
      this->sectorVector.clear();

      for (VXDSegmentCell* aCell : this->totalCellVector) {  //segments, Hits and TrackCandidates can be deleted
        delete  aCell;
      }
      this->totalCellVector.clear();
      this->activeCellList.clear();

      for (VXDTFHit* aHit : this->hitVector) {
        delete aHit;
      }
      this->hitVector.clear(); // used for normal passes
      this->fullHitsVector.clear(); // used for baseline-passes

      /// deleting TCs will be done in cleanEvent from the VXDTFModule
//       for (VXDTFTrackCandidate* aTC : this->tcVector) {
//  delete aTC;
//       }
      this->tcVector.clear();
    }



    /** ** filled once ** **/

    unsigned int centerSector; /**< ID of center sector, needed for clean-up */
    MapOfSectors sectorMap; /**< carries the whole lookup-table including the information of which sectors can be combined */

    std::string sectorSetup; /**< name of setup, needed e.g. for XML-readout */
    std::string additionalInfo; /**< carries some extra info about current setup/pass */
    TVector3 origin; /**< stores the chosen origin (position of assumed primary vertex) */
    std::string
    chosenDetectorType; /**< same as detectorType, but fully written ('SVD','PXD' or 'TEL' allowed), needed during xml and for logging messages */
    bool m_isFilterByDistance2OriginActivated; /**< if true, the sectors are not sorted by layerID but by distance to origin values */
    bool useSVDHits; /**< if true, this pass uses SVD hits */
    bool usePXDHits; /**< if true, this pass uses PXD hits */
    bool useTELHits; /**< if true, this pass uses TEL hits */
    double reserveHitsThreshold; /**< value between 0 (0%) and 1 (100%), defines how many tcs are allowed to reserve their clusters which forbids other TCs to use them too */
    unsigned short int
    highestAllowedLayer; /**< needed for e.g. hitfinding. This value excludes Layernumbers higher than set value. (interesting for low momentum tracks)  */
    int numTotalLayers;  /**< needed e.g. for neuronal network. This value allows calculation of maximum track length (noncurling), carries implicit information about detectorType, while 'highestAllowedLayer' does not know whether its a SVD or VXD setup */
    int minLayer; /**< lowest layer considered for TCC */
    int minState; /**< lowest state considered for seeds during TCC */
    double magneticFieldStrength; /**< the strength of the magnetic field in Tesla */
    double generalTune; /**< allows stretching of all filterValues at once*/


    /** soon to come (maybe even layer-specific): **/

    std::vector<double> secConfigU;  /**< defines subdivition of sensors U */
    std::vector<double> secConfigV; /**< defines subdivition of sensors V */


    /** for segFinder, compares 2 hits**/
    PassDataTypedef::Filter
    distance3D; /**< carries information about the filter 'distance3D', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    distanceXY; /**< carries information about the filter 'distanceXY', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    distanceZ; /**< carries information about the filter 'distanceZ', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    slopeRZ; /**< carries information about the filter 'slopeRZ', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    normedDistance3D; /**< carries information about the filter 'normedDistance3D', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    alwaysTrue2Hit; /**< carries information about the filter for test which always approves the input files, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    alwaysFalse2Hit; /**< carries information about the filter test which always discards the input files, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    random2Hit; /**< carries information about the filter test which randomly discards the input files, type pair<bool isActivated, double tuningParameter> */
    int activatedSegFinderTests; /**< counts number of tests activated for segFinder */


    /** for segFinder in high occupancy mode, compares 2+1 hits (using origin as third hit) */

    PassDataTypedef::Filter anglesHighOccupancy3D; /**< exactly the same as 'angles3D' but for high occupancy cases */
    PassDataTypedef::Filter anglesHighOccupancyXY; /**< exactly the same as 'anglesXY' but for high occupancy cases */
    PassDataTypedef::Filter anglesHighOccupancyRZ; /**< exactly the same as 'anglesRZ' but for high occupancy cases */
    PassDataTypedef::Filter deltaSlopeHighOccupancyRZ; /**< exactly the same as 'deltaSlopeRZ' but for high occupancy cases */
    PassDataTypedef::Filter pTHighOccupancy; /**< exactly the same as 'pT' but for high occupancy cases */
    PassDataTypedef::Filter distanceHighOccupancy2IP; /**< exactly the same as 'distance2IP' but for high occupancy cases */
    PassDataTypedef::Filter helixParameterHighOccupancyFit; /**< exactly the same as 'helixFit' but for high occupancy cases */
    int activatedHighOccupancySegFinderTests; /**< counts number of tests activated for segFinder in HighOccupancy mode */


    /** for nbFinder, compares 3 hits **/

    PassDataTypedef::Filter
    angles3D; /**< carries information about the filter 'angles3D', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    anglesXY; /**< carries information about the filter 'anglesXY', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    anglesRZ; /**< carries information about the filter 'anglesRZ', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    deltaSlopeRZ; /**< carries information about the filter 'deltaSlopeRZ', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter pT; /**< carries information about the filter 'pT', same type as others */
    PassDataTypedef::Filter
    distance2IP; /**< carries information about the filter 'distance2IP', type pair<bool isActivated, double tuningParameter>  in XY-plane, determines center of projection circle of the track and measures the residual between distance IP <-> circleCenter minus circleRadius, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    helixParameterFit; /**< carries information about the filter 'helixFit', same type as others. It uses the fact that a helix projection to the xy-plane forms a circle. Any angle of a segment of a circle divided by its deltaZ-value has to be the same value. Therefore any three hits lying on the same helix can be compared this way, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    deltaSOverZ; /**< calculates the helixparameter describing the deviation in arc length per unit in z, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    deltaSlopeZOverS; /**< compares the "slopes" z over arc length. calcDeltaSlopeZOverS is invariant under rotations in the r-z plane, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    alwaysTrue3Hit; /**< carries information about the filter for test which always approves the input files, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    alwaysFalse3Hit; /**< carries information about the filter test which always discards the input files, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    random3Hit; /**< carries information about the filter test which randomly discards the input files, type pair<bool isActivated, double tuningParameter> */
    int activatedNbFinderTests; /**< counts number of tests activated for nbFinder */


    /** for nbFinder in high occupancy mode, compares 3+1 hits (using origin as fourth hit) */

    PassDataTypedef::Filter deltaPtHighOccupancy; /**< exactly the same as 'deltaPt' but for high occupancy cases */
    PassDataTypedef::Filter deltaDistanceHighOccupancy2IP; /**< exactly the same as 'deltaDistance2IP' but for high occupancy cases */
    int activatedHighOccupancyNbFinderTests; /**< counts number of tests activated for nbFinder */


    /** for TCC filter, compares 4 hits **/

    PassDataTypedef::Filter
    zigzagXY; /**< carries information about the filter 'zigZagXY', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
    PassDataTypedef::Filter
    zigzagXYWithSigma; /**< checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns true, if they are ziggzagging. This functions uses the sigma of the positionInfos to consider also approximately straight tracks as not zigg-zagging, type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
    PassDataTypedef::Filter
    zigzagRZ; /**< carries information about the filter 'zigZagRZ', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
    PassDataTypedef::Filter
    deltaPt; /**< carries information about the filter 'deltaPt', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    deltaDistance2IP; /**< carries information about the filter 'deltaDistance2IP', type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    circleFit; /**< carries information about the filter 'circleFit', type pair<bool isActivated, doubletuningParameter> , here the tuningparameter is currently used to store the global chi2 threshold value. */
    PassDataTypedef::Filter
    alwaysTrue4Hit; /**< carries information about the filter for test which always approves the input files, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    alwaysFalse4Hit; /**< carries information about the filter test which always discards the input files, type pair<bool isActivated, double tuningParameter> */
    PassDataTypedef::Filter
    random4Hit; /**< carries information about the filter test which randomly discards the input files, type pair<bool isActivated, double tuningParameter> */
    int activatedTccFilterTests; /**< counts number of tests activated for tcc filter */

    SegFinderFilters twoHitFilterBox; /**< contains all the two hit filters needed by the segFinder */
    NbFinderFilters threeHitFilterBox; /**< contains all the three hit filters needed by the nbFinder */
    TcFourHitFilters fourHitFilterBox; /**< contains all the four hit filters needed by the post-ca-Filter */
    TrackletFilters trackletFilterBox; /**< contains all the four-or-more hit filters needed by the post-ca-Filter */


    /** filled and resetted each event **/

    OperationVectorOfActivatedSectors sectorVector; /**< carries pointers to sectors which are used in current event */
    ActiveSegmentsOfEvent activeCellList; /**< list of active cells, dead ones get kicked (therefore it's a list) */
    TotalSegmentsOfEvent totalCellVector;   /**< This vector contains throughout the whole event all segments found. It is needed since
    * VXDTFHits are not allowed to carry pointers to Segments any more. As a cheap replacement, indices of this vector are stored in
    * each hit. We still need this link since until the neighbourfinder did his work, no segmentCell does know its neighbour (the name
    * explains itself ;) ).  Therefore each hit carries the indices of its inner and outer neighbours so the neighbourfinder can easily
    * find neighbouring segments. */
    HitsOfEvent hitVector; /**< carries total number of hits of current event. */
    std::vector<VXDTFHit> fullHitsVector; /**< carries hits of event (no pointers), used by baselineTF */
    TCsOfEvent tcVector; /**< carries track candidates of current pass */

  protected:
    /** storing direct pointers to each friend for all sectors should only be called by importSectorMap - should be called only once! */
    void linkSectorsToFriends()
    {
      // first collecting all sectorPointers
      std::vector<VXDSector*> sectors;
      for (auto& secMapEntry : sectorMap) {
        sectors.push_back(secMapEntry.second);
      }

      std::vector<unsigned int> friendIndices;
      // now reentering each sector for storing friendPointers
      for (auto* aSector : sectors) {
        friendIndices = aSector->getFriends();
        for (unsigned int anIndex : friendIndices) {
          auto iterator = sectorMap.find(anIndex);
          if (iterator != sectorMap.end()) {
            aSector->addFriendPointer(iterator->second);
          }
        }
      }
    }
  }; //
} //Belle2 namespace

