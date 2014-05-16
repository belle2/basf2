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

// #include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <utility>      // std::pair
#include <TVector3.h>
// #include <TMatrixD.h>
// #include <Eigen/Dense>
// #include <iostream>
// #include <sstream> // stringstream
#ifndef __CINT__
#include <boost/unordered_map.hpp>
#endif

#include "VXDSector.h"
#include "VXDTFHit.h"
#include "VXDSegmentCell.h"
#include "tracking/vxdCaTracking/VXDTFTrackCandidate.h"

#include "tracking/vxdCaTracking/SegFinderFilters.h"
#include "tracking/vxdCaTracking/NbFinderFilters.h"
#include "tracking/vxdCaTracking/TcFourHitFilters.h"
#include "tracking/vxdCaTracking/TrackletFilters.h"

namespace Belle2 {


  // some typedefs needed for clarity:
  typedef std::pair< double, double> CutoffValue; /**< .first is minValue, .second is maxValue */
  typedef std::pair<unsigned int, CutoffValue> CutoffPack; /**< .first is code of filter, .second is CutoffValue */
  typedef std::vector< CutoffPack > FriendValue; /**< stores all Cutoffs */
  typedef std::pair<unsigned int, FriendValue > FriendPack; /**< .first is secID of current Friend, second is FriendValue */
  typedef std::vector< FriendPack > SectorValue; /**< stores all Friends */
  typedef std::pair<unsigned int, SectorValue> SectorPack; /**< .first is secID of current sector, second is SectorValue */
  typedef std::vector < SectorPack > SecMapCopy; /**< stores all Sectors */

  typedef std::pair<bool, double> Filter; /**< defines whether current filter is allowed (.first) and whether the cutoffvalues shall be tuned (.second).  */

  typedef boost::unordered_map<unsigned int, VXDSector*> MapOfSectors; /**< stores whole sectorMap used for storing cutoffs */
  typedef std::pair<unsigned int, MapOfSectors::iterator> SectorNameAndPointerPair; /**< we are storing the name of the sector (which is encoded into an int) to be able to sort them! */
  typedef std::list<SectorNameAndPointerPair> OperationSequenceOfActivatedSectors; /**< contains all active sectors, can be sorted by name (first entry) */
  typedef std::list<VXDSegmentCell*> ActiveSegmentsOfEvent; /**< is list since random deleting processes are needed */
  typedef std::vector<VXDSegmentCell*> TotalSegmentsOfEvent; /**< is vector since no entries are deleted and random access is needed  */
  typedef std::vector<VXDTFHit*> HitsOfEvent; /**< contains pointers to all hits of event */
  typedef std::vector<VXDTFTrackCandidate*> TCsOfEvent; /**< contains all track candidates of event */

  /** all the information of a pass is stored within a copy of that class. This allows grouping of that information. WARNING: at the moment, this class is still a hybrid between a struct and a class. A full set of setters and getters is missing yet and since there are so many member variables, it will probably never implemented...*/
  class PassData {
  public:
    /**
     * the following function retrieves a map of sectors used by VXDTF and CKFTF out of rawData given to it.
     * input is the raw sector map which is a vector of sectors.
     * output is a pair of counters, where:
     * .first is counting the total number of sectorFriends (connected inner sectors) and
     * .second is counting the total number of cutoffs stored
     */
    std::pair<int, int> importSectorMap(const SecMapCopy& rawSecMap);


    /** fills in standard-settings for all filters */
    void activateAllFilters();

    /** for streching the limits of the cutoff values for finetuning */
    double addExtraGain(double prefix, double cutOff, double specialTune) {
      if (cutOff > 0.) { return cutOff + prefix * cutOff * this->generalTune + prefix * cutOff * specialTune; }
      return cutOff - prefix * cutOff * this->generalTune - prefix * cutOff * specialTune;
    }



    /** clears all event-wise data **/
    void cleanPass() {
      for (SectorNameAndPointerPair & aSector : this->sectorSequence) { // sectors stay alive, therefore only reset!
        aSector.second->second->resetSector();
      }
      this->sectorMap.find(this->centerSector)->second->resetSector(); // doing the same for the virtual sector which is not in the operation sequence
      this->sectorSequence.clear();

      for (VXDSegmentCell * aCell : this->totalCellVector) { //segments, Hits and TrackCandidates can be deleted
        delete  aCell;
      }
      this->totalCellVector.clear();
      this->activeCellList.clear();

      for (VXDTFHit * aHit : this->hitVector) {
        delete  aHit;
      }
      this->hitVector.clear(); // used for normal passes
      this->fullHitsVector.clear(); // used for baseline-passes

      this->tcVector.clear();
    }



    /** ** filled once ** **/

    unsigned int centerSector; /**< ID of center sector, needed for clean-up */
    MapOfSectors sectorMap; /**< carries the whole lookup-table including the information of which sectors can be combined */

    std::string sectorSetup; /**< name of setup, needed e.g. for XML-readout */
    std::string additionalInfo; /**< carries some extra info about current setup/pass */
    TVector3 origin; /**< stores the chosen origin (position of assumed primary vertex) */
    std::string chosenDetectorType; /**< same as detectorType, but fully written ('SVD','PXD' or 'VXD'), needed during xml and for logging messages */
    int detectorType; /**< PXD = 0 , SVD = 1, VXD = -1 */
    double reserveHitsThreshold; /**< value between 0 (0%) and 1 (100%), defines how many tcs are allowed to reserve their clusters which forbids other TCs to use them too */
    short int highestAllowedLayer; /**< needed for e.g. hitfinding. This value excludes Layernumbers higher than set value. (interesting for low momentum tracks)  */
    int numTotalLayers;  /**< needed e.g. for neuronal network. This value allows calculation of maximum track length (noncurling), carries implicit information about detectorType, while 'highestAllowedLayer' does not know whether its a SVD or VXD setup */
    int minLayer; /**< lowest layer considered for TCC */
    int minState; /**< lowest state considered for seeds during TCC */
    double magneticFieldStrength; /**< the strength of the magnetic field in Tesla */
    double generalTune; /**< allows stretching of all filterValues at once*/

    /** soon to come (maybe even layer-specific): **/
    std::vector<double> secConfigU;  /**< defines subdivition of sensors U */
    std::vector<double> secConfigV; /**< defines subdivition of sensors V */

    /** for segFinder, compares 2 hits**/
    Filter distance3D; /**< carries information about the filter 'distance3D', type pair<bool isActivated, double tuningParameter> */
    Filter distanceXY; /**< carries information about the filter 'distanceXY', type pair<bool isActivated, double tuningParameter> */
    Filter distanceZ; /**< carries information about the filter 'distanceZ', type pair<bool isActivated, double tuningParameter> */
    Filter slopeRZ; /**< carries information about the filter 'slopeRZ', type pair<bool isActivated, double tuningParameter> */
    Filter normedDistance3D; /**< carries information about the filter 'normedDistance3D', type pair<bool isActivated, double tuningParameter> */
    int activatedSegFinderTests; /**< counts number of tests activated for segFinder */
    /** for segFinder in high occupancy mode, compares 2+1 hits (using origin as third hit) */
    Filter anglesHighOccupancy3D; /**< exactly the same as 'angles3D' but for high occupancy cases */
    Filter anglesHighOccupancyXY; /**< exactly the same as 'anglesXY' but for high occupancy cases */
    Filter anglesHighOccupancyRZ; /**< exactly the same as 'anglesRZ' but for high occupancy cases */
    Filter deltaSlopeHighOccupancyRZ; /**< exactly the same as 'deltaSlopeRZ' but for high occupancy cases */
    Filter pTHighOccupancy; /**< exactly the same as 'pT' but for high occupancy cases */
    Filter distanceHighOccupancy2IP; /**< exactly the same as 'distance2IP' but for high occupancy cases */
    Filter helixHighOccupancyFit; /**< exactly the same as 'helixFit' but for high occupancy cases */
    int activatedHighOccupancySegFinderTests; /**< counts number of tests activated for segFinder in HighOccupancy mode */

    /** for nbFinder, compares 3 hits **/
    Filter angles3D; /**< carries information about the filter 'angles3D', type pair<bool isActivated, double tuningParameter> */
    Filter anglesXY; /**< carries information about the filter 'anglesXY', type pair<bool isActivated, double tuningParameter> */
    Filter anglesRZ; /**< carries information about the filter 'anglesRZ', type pair<bool isActivated, double tuningParameter> */
    Filter deltaSlopeRZ; /**< carries information about the filter 'deltaSlopeRZ', type pair<bool isActivated, double tuningParameter> */
    Filter pT; /**< carries information about the filter 'pT', same type as others */
    Filter distance2IP; /**< carries information about the filter 'distance2IP', type pair<bool isActivated, double tuningParameter>  in XY-plane, determines center of projection circle of the track and measures the residual between distance IP <-> circleCenter minus circleRadius */
    Filter helixFit; /**< carries information about the filter 'helixFit', same type as others. It uses the fact that a helix projection to the xy-plane forms a circle. Any angle of a segment of a circle divided by its deltaZ-value has to be the same value. Therefore any three hits lying on the same helix can be compared this way */
    int activatedNbFinderTests; /**< counts number of tests activated for nbFinder */
    /** for nbFinder in high occupancy mode, compares 3+1 hits (using origin as fourth hit) */
    Filter deltaPtHighOccupancy; /**< exactly the same as 'deltaPt' but for high occupancy cases */
    Filter deltaDistanceHighOccupancy2IP; /**< exactly the same as 'deltaDistance2IP' but for high occupancy cases */
    int activatedHighOccupancyNbFinderTests; /**< counts number of tests activated for nbFinder */

    /** for TCC filter, compares 4 hits **/
    Filter zigzagXY; /**< carries information about the filter 'zigZagXY', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
    Filter zigzagRZ; /**< carries information about the filter 'zigZagRZ', type pair<bool isActivated, double tuningParameter> , here the tuningparameter is currently ignored, possible future use is reserved for defining uncertainties because of measurement errors */
    Filter deltaPt; /**< carries information about the filter 'deltaPt', type pair<bool isActivated, double tuningParameter> */
    Filter deltaDistance2IP; /**< carries information about the filter 'deltaDistance2IP', type pair<bool isActivated, double tuningParameter> */
    Filter circleFit; /**< carries information about the filter 'circleFit', type pair<bool isActivated, doubletuningParameter> , here the tuningparameter is currently used to store the global chi2 threshold value. */
    int activatedTccFilterTests; /**< counts number of tests activated for tcc filter */

    SegFinderFilters twoHitFilterBox; /**< contains all the two hit filters needed by the segFinder */
    NbFinderFilters threeHitFilterBox; /**< contains all the three hit filters needed by the nbFinder */
    TcFourHitFilters fourHitFilterBox; /**< contains all the four hit filters needed by the post-ca-Filter */
    TrackletFilters trackletFilterBox; /**< contains all the four-or-more hit filters needed by the post-ca-Filter */

    /** filled and resetted each event **/
    OperationSequenceOfActivatedSectors sectorSequence; /**< carries pointers to sectors which are used in current event */
    ActiveSegmentsOfEvent activeCellList; /**< list of active cells, dead ones get kicked (therefore it's a list) */
    TotalSegmentsOfEvent totalCellVector;   /**< This vector contains throughout the whole event all segments found. It is needed since
    * VXDTFHits are not allowed to carry pointers to Segments any more. As a cheap replacement, indices of this vector are stored in
    * each hit. We still need this link since until the neighbourfinder did his work, no segmentCell does know its neighbour (the name
    * explains itself ;) ).  Therefore each hit carries the indices of its inner and outer neighbours so the neighbourfinder can easily
    * find neighbouring segments. */
    HitsOfEvent hitVector; /**< carries total number of hits of current event. atm only used for .size() and final delete of hits at end of event, can we remove it? */
    std::vector<VXDTFHit> fullHitsVector; /**< carries hits of event (no pointers), used by baselineTF */
    TCsOfEvent tcVector; /**< carries track candidates of current pass */
  }; //
} //Belle2 namespace
