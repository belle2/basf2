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

// stl:
#include <string>

// root
#include <TVector3.h>
#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** The Sector Map Container used by the Filter Calculator Module
   * This class is needed for calculating a sectorMap for the VXDTF.
   * This Class is root-container-compatible and is designed to be reopened several times
   */
  class VXDTFRawSecMap: public TObject {
  public:
    typedef std::list< double > CutoffValues; /**< contains all cutoff-Values collected during run, is a list since it has to be sorted in the end */
    typedef std::pair<unsigned int, CutoffValues> CutoffPack; /**< .first is code of filter, .second is CutoffValue */
    typedef std::vector< CutoffPack > FriendValues; /**< stores all Cutoffs */
    typedef std::pair<unsigned int, FriendValues > FriendPack; /**< .first is secID of current Friend, second is FriendValue */
    typedef std::vector< FriendPack > SectorValues; /**< stores all Friends */
    typedef std::pair<unsigned int, SectorValues> SectorPack; /**< .first is secID of current sector, second is SectorValue */
    typedef std::vector < SectorPack > StrippedRawSecMap; /**< stores all Sectors and a raw version of the data (no calculated cutoffs yet)*/
    typedef std::vector< unsigned int > IDVector; /**< stores IDs of sectors or friends */
    typedef std::pair<unsigned int, double> SectorDistance; /**< stores distance to origin (.second) for sector (.first) */
    typedef std::vector< SectorDistance > SectorDistancesMap; /**< stores vector of SectorDistanceInfo */
    // well, without typedef this would be:
    // vector< pair< unsigned int, vector< pair< unsigned int, vector< pair< unsigned int, vector<double> > > > > > > .... hail typedefs -.-

    /** Default constructor for the ROOT IO. */
    VXDTFRawSecMap():
      m_magneticFieldStrength(1.5),
      m_lowPt(0.),
      m_highPt(0.),
      m_minDistance2origin(-1),
      m_maxDistance2origin(-1),
      m_sortByDistance2origin(false),
      m_smallSampleThreshold(1),
      m_minSampleThreshold(1),
      m_smallStretchFactor(0.),
      m_stretchFactor(0.),
      m_removeDeadSectorChains(true) {
      m_maxLayerLevelDifference.first = true;
      m_maxLayerLevelDifference.second = 2;
      m_rareSectorCombinations.first = false;
      m_rareSectorCombinations.second = 0.0;
    }


    bool operator<(const VXDTFRawSecMap& b)  const {
      bool result = ((m_nameOfSecMap < b.getMapName())
                     and (m_detectorType < b.getDetectorType())
                     and (m_sectorConfigU < b.getSectorConfigU())
                     and (m_sectorConfigV < b.getSectorConfigV())
//                    and (m_origin < b.getOrigin()
                     and (m_magneticFieldStrength < b.getMagneticFieldStrength())
                     and (m_lowPt < b.getLowerMomentumThreshold())
                     and (m_highPt < b.getHigherMomentumThreshold()));
      return result;
    } /**< overloaded '<'-operator for sorting algorithms, m_sectorMap is not part of that step, since we want to use that operator to find compatible SecMaps */



    bool operator==(const VXDTFRawSecMap& b) const {
      bool result = ((m_nameOfSecMap == b.getMapName())
                     and (m_detectorType == b.getDetectorType())
                     and (m_sectorConfigU == b.getSectorConfigU())
                     and (m_sectorConfigV == b.getSectorConfigV())
//                    and (m_origin == b.getOrigin()
                     and (m_magneticFieldStrength == b.getMagneticFieldStrength())
                     and (m_lowPt == b.getLowerMomentumThreshold())
                     and (m_highPt == b.getHigherMomentumThreshold()));
      return result;
    } /**< overloaded '=='-operator for sorting algorithms */



    bool operator>(const VXDTFRawSecMap& b)  const {
      bool result = ((m_nameOfSecMap > b.getMapName())
                     and (m_detectorType > b.getDetectorType())
                     and (m_sectorConfigU > b.getSectorConfigU())
                     and (m_sectorConfigV > b.getSectorConfigV())
//                    and (m_origin > b.getOrigin()
                     and (m_magneticFieldStrength > b.getMagneticFieldStrength())
                     and (m_lowPt > b.getLowerMomentumThreshold())
                     and (m_highPt > b.getHigherMomentumThreshold()));
      return result;
    } /**< overloaded '>'-operator for sorting algorithms */



    /** getter - returns full sectorMapInformation */
    VXDTFRawSecMapTypedef::StrippedRawSecMap& getSectorMap() { return m_sectorMap; }



    /** getter - returns information for each sector carrying the distance between chosen origin and the center of the sector plane */
    VXDTFRawSecMapTypedef::SectorDistancesMap& getDistances() { return m_dist2OriginMap; }



    /** getter - returns name of sectorMap */
    std::string getMapName() const { return m_nameOfSecMap; }



    /** getter - returns name of detectorType */
    std::string getDetectorType() const { return m_detectorType; }



    /** getter - returns config of the sectors in U direction */
    std::vector<double> getSectorConfigU() const { return m_sectorConfigU; }



    /** getter - returns config of the sectors in V direction */
    std::vector<double> getSectorConfigV() const { return m_sectorConfigV; }



    /** getter - returns the position of the assumed primary vertex */
    TVector3 getOrigin() const { return m_origin; }



    /** getter - returns the strength of magnetic field in Tesla */
    double getMagneticFieldStrength() const { return m_magneticFieldStrength; }



    /** getter - returns lower threshold for transverse momentum stored in that map */
    double getLowerMomentumThreshold() const { return m_lowPt; }



    /** getter - returns higher threshold for transverse momentum stored in that map */
    double getHigherMomentumThreshold() const { return m_highPt; }



    /** returns threshold for big samples, if sampleSize is smaller than threshold, the other value of 'getMinSampleThreshold()' will be used */
    double getSampleThreshold() { return m_smallSampleThreshold; }



    /** if sample size of a given sector-combination (using 'calcCutoffs(sector-combination)') is smaller than returned value, it gets rejected */
    double getMinSampleThreshold() { return m_minSampleThreshold; }



    /** returns size of stored sectorMap */
    int size() { return m_sectorMap.size(); }



    /** returns size of stored VXDTFRawSecMapTypedef::StrippedRawSecMap, same as size() */
    int getNumOfSectors() { return size(); }



    /** returns total number of friends stored in rawSecMap */
    int getNumOfFriends() { return getNumOfFriends(m_sectorMap); }



    /** returns total number of friends stored in rawSecMap */
    int getNumOfValues() { return getNumOfValues(m_sectorMap); }



    /** getter - using cutoffQuantiles set with 'setCutoffQuantiles' to determine cutoffs of given sample. If returned values are both  == 0, then sample was rejected */
    std::pair<double, double> calcCutoffs(VXDTFRawSecMapTypedef::CutoffValues& sample) {
      if (int(sample.size() < m_minSampleThreshold)) { return std::make_pair(0, 0); } // catching case where sample size is too small

      sample.sort(); // in theory should not be necessary since all values have been sorted during import. But we want to be sure, that there are no errors on that part (and speed is not an issue for creating sectorMaps)

      if (int(sample.size()) < m_smallSampleThreshold) { return calcCutoff(sample, m_smallCutoffQuantiles, m_smallStretchFactor); } // case of small but allowed sample size

      return calcCutoff(sample, m_cutoffQuantiles, m_stretchFactor); // normal case
    }



    /** returns minimal accepted distance for sectors to the origin */
    double getMinDistance2origin() { return m_minDistance2origin; }



    /** returns maximum accepted distance for sectors to the origin*/
    double getMaxDistance2origin() { return m_maxDistance2origin; }



    /** if true, not only sectors having no friends after first filtering iteration are deleted but also all combinations where dead sectors were friends. If these combinations can lead to kill more sectors and therefore the map gets holey but clean of non-existing cases */
    bool isRemovingDeadSectorChainsAllowed() { return m_removeDeadSectorChains; }



    /** checks sector and friendSector for sector distance to origin based filter cases. If friend fails test, its secID is returned. -1 else */
    int filterDistanceBased(unsigned int mainSecID, VXDTFRawSecMapTypedef::FriendPack& aCombination);



    /** if true hits are sorted by distance to origin and not by layerID */
    bool isFilterByDistance2OriginActivated() { return m_sortByDistance2origin; }



    /** checks sector and friendSector for layer based filter cases. If friend fails test, its secID is returned. -1 else */
    int filterLayerBased(unsigned int mainSecID, VXDTFRawSecMapTypedef::FriendPack& aCombination);



    /** printing for each sector its complete friend list and their number of cutoffTypes stored */
    void printDetailedInfo() { printDetailedInfo(m_sectorMap); }



    /** INFO: use this member only after importing all the other ones! This member checks each sector whether it has a friend on the same layer. If there is one, it does security checks (friends on same sensor are forbidden) and updates the subLayerID. After that, it updates the infos of the friends too. */
    void repairSecMap();



    /** set true if you want to sort secMap by distance instead of layerIDs */
    void setFilterByDistance2Origin(bool val) { m_sortByDistance2origin = val; }



    /** This member checks all entries of the sectorMap and stores all the layerIDs of the occuring sectors */
    VXDTFRawSecMapTypedef::IDVector getLayersOfSecMap();



    /** setter - add new map to current one. If current one is empty, it will get replaced by new one */
    void addSectorMap(VXDTFRawSecMapTypedef::StrippedRawSecMap& newMap);



    /** setter - set name of sectorMap */
    void setMapName(std::string newName) { m_nameOfSecMap = newName; }



    /** setter - set name of detectorType */
    void setDetectorType(std::string newName) { m_detectorType = newName; }



    /** setter - defines the config of the sectors in U direction, value is valid for each sensor of chosen detector setup, minimum 2 values, all values: 0.0 <= x <= 1.0 */
    void setSectorConfigU(std::vector<double> newUconfig) { m_sectorConfigU = newUconfig; }



    /** setter - defines the config of the sectors in V direction, value is valid for each sensor of chosen detector setup, minimum 2 values, all values: 0.0 <= x <= 1.0 */
    void setSectorConfigV(std::vector<double> newVconfig) { m_sectorConfigV = newVconfig; }



    /** setter - sets the position of the assumed primary vertex */
    void setOrigin(TVector3 origin) { m_origin = origin; }



    /** setter - sets the strength of magnetic field in Tesla, standard is 1.5T */
    void setMagneticFieldStrength(double mField) { m_magneticFieldStrength = mField; }



    /** setter - set lower threshold for transverse momentum stored in that map */
    void setLowerMomentumThreshold(double newInfo) { m_lowPt = newInfo; }



    /** setter - add new distance map (nformation for each sector carrying the distance between chosen origin and the center of the sector plane) to current one. If current one is empty, it will get replaced by new one */
    void addDistances(VXDTFRawSecMapTypedef::SectorDistancesMap& aMap);



    /** setter - set higher threshold for transverse momentum stored in that map */
    void setHigherMomentumThreshold(double newInfo) { m_highPt = newInfo; }



    /** sets quantiles for cutoffs (value 0-1): .first is lower quantile for cutoff, .second is higher quantile */
    void setCutoffQuantiles(std::pair<double, double> quantiles) {
      checkSmallerThan(quantiles.first, 0.); checkSmallerThan(quantiles.second, 0.);
      checkBiggerThan(quantiles.first, 1.); checkBiggerThan(quantiles.second, 1.);
      m_cutoffQuantiles = quantiles;
    }



    /** same as for 'setCutoffQuantiles(pairOfQuantiles)' but for sector-combinations with small sample size -> quantiles for cutoffs (value 0-1): .first is lower quantile for cutoff, .second is higher quantile */
    void setSmallCutoffQuantiles(std::pair<double, double> quantiles) {
      checkSmallerThan(quantiles.first, 0.); checkSmallerThan(quantiles.second, 0.);
      checkBiggerThan(quantiles.first, 1.); checkBiggerThan(quantiles.second, 1.);
      m_smallCutoffQuantiles = quantiles;
    }



    /** sets threshold for big samples, if sampleSize is smaller than given threshold, the other set of 'setMinSampleThreshold(aThreshold)' will be used */
    void setSampleThreshold(double threshold)
    { checkSmallerThan(threshold, 0.); m_smallSampleThreshold = threshold; }



    /** if sample size of a given sector-combination (using 'calcCutoffs(sector-combination)') is smaller than this value, it gets rejected */
    void setMinSampleThreshold(double threshold)
    { checkSmallerThan(threshold, 0.); m_minSampleThreshold = threshold; }



    /** factor by which the values can be stretched when having a normal sample size. The value will be increased(for max cutoffs)/decreased(for min cutoffs) by factor times the original value -> if factor == 0 -> no stretching */
    void setStretchFactor(double factor) { m_stretchFactor = factor; }



    /** factor by which the values can be stretched when having a normal sample size. The value will be increased(for max cutoffs)/decreased(for min cutoffs) by factor times the original value -> if factor == 0 -> no stretching */
    void setSmallStretchFactor(double factor) { m_smallStretchFactor = factor; }



    /** use this member if you want to steer whether sector-friend-combinations with several layers in between are allowed or not. First parameter activates this filter, second is threshold in numbers of layer in difference. If value is e.g. 2, then sector-friend combinations which are more than 2 layers away will be filtered (L6->L4 okay, but L6->L3 gets kicked) */
    void setMaxLayerLevelDifference(bool checkDiff, int threshold) { m_maxLayerLevelDifference = std::make_pair(checkDiff, threshold); }



    /** use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not. first parameter is whether you want to filter these combinations or not, second is (if first parameter is true) the threshold for filter. 100% = 1. 1% = 0.01% ... */
    void setRareSectorCombinations(bool checkRareness, double threshold) { m_rareSectorCombinations = std::make_pair(checkRareness, threshold); }



    /** if true, not only sectors having no friends after first filtering iteration are deleted but also all combinations where dead sectors were friends. If these combinations can lead to kill more sectors and therefore the map gets holey but clean of non-existing cases */
    void setRemoveDeadSectorChains(bool allowThis) { m_removeDeadSectorChains = allowThis; }


    /** set minimal accepted distance for sectors to the origin */
    void setMinDistance2origin(double newVal) { m_minDistance2origin = newVal; }


    /** set maximum accepted distance for sectors to the origin*/
    void setMaxDistance2origin(double newVal) { m_maxDistance2origin = newVal; }



    template<class Tmpl>
    static void checkSmallerThan(Tmpl& number, Tmpl threshold) { if (number < threshold) { number = threshold; } } /**< member - checks if given value is < threshold, if it is, reset to threshold */



    template<class Tmpl>
    static void checkBiggerThan(Tmpl& number, Tmpl threshold) { if (number > threshold) { number = threshold; } } /**< member - checks if given value is > threshold, if it is, reset to threshold */



  protected:
    /** sorts all values for each cutoffType for each sector stored (sorts sectors too) */
    void sortMap(VXDTFRawSecMapTypedef::StrippedRawSecMap& newMap);



    /** internal member counting total number of friends stored in rawSecMap and returns result */
    int getNumOfFriends(VXDTFRawSecMapTypedef::StrippedRawSecMap& newMap) {
      int result = 0;
      for (int i = 0; i < int(newMap.size()); ++i) {
        result += newMap[i].second.size();
      }
      return result;
    }



    /** internal member printing for each sector its complete friend list and their number of cutoffTypes stored */
    void printDetailedInfo(VXDTFRawSecMapTypedef::StrippedRawSecMap& secMap);



    /** internal member counting total number of friends stored in rawSecMap and returns result */
    int getNumOfValues(VXDTFRawSecMapTypedef::StrippedRawSecMap& secMap);



    /** internal member - using quantiles to determine cutoffs of given sample.
     *
     * If returned values are both  == 0, then sample was rejected.
     * first parameter: the sample of values for which the cutoffs shall be determined, has to be sorted beforehand.
     * second parameter: the lower (.first) and the upper quantile (.second) to determine the cutoffs of given sample. values have to be between 0 and 1. WARNING check
     * third parameter: the stretchFactor scales the cutoffs by its value (increases value for higher and decreases them for lower cutoffs). WARNING check
     **/
    VXDTFSecMapTypedef::CutoffValue calcCutoff(VXDTFRawSecMapTypedef::CutoffValues& sample, VXDTFSecMapTypedef::CutoffValue& quantiles, double stretchFactor);



    double addExtraGain(double cutOff, double gain) {
      if (cutOff > 0.) { return cutOff + cutOff * gain; }
      return cutOff - cutOff * gain;
    } /**< changes value (first parameter) by gain (second parameter). The value will be increased(for positive gain)/decreased(for negative gain) by gain times the original value */



    /** checks sector-combination whether it is allowed or not (checks: isSameSensor, isUncommonCombination, isTooManyLayersAway), return value is true, if if main sector of the pack shall get a subLayerID-upgrade */
    bool filterBadSectorCombis(VXDTFRawSecMapTypedef::SectorPack& aSector);



    template<class Tmpl>
    void searchForBadEntries(Tmpl& entryVector, VXDTFRawSecMapTypedef::IDVector& badIDs) {
      // now kick bad entries (friends/sectors) (INFO that following part is pretty inefficient towards time consumption, if anyone thinks about optimizing this, using a list instead of a vector could do the job...)
      typename Tmpl::iterator entryIt = entryVector.begin(); // typename is needed for the compiler recognizing this as a template
      VXDTFRawSecMapTypedef::IDVector::iterator badIDit = badIDs.begin();
      while (entryIt != entryVector.end()) { // enters infinite loop when iterator runs rampage, used as a primitive boundary check
        bool foundID = false; // is set true, if a badID has been found

        for (badIDit = badIDs.begin() ; badIDit != badIDs.end() ; ++badIDit) { // enters infinite loop when iterator runs rampage, could be done in a safer way with c++11 for ( : )-loop, but not supported yet by root
          if ((*badIDit) == entryIt->first) {
            foundID = true;
            break;
          }
        }

        if (foundID == true) {
          entryIt = entryVector.erase(entryIt); // entryIt points at first entry after erased one
        } else { ++entryIt; }
      }
    }/**< compares secID of each given sector/friend with given list of bad IDs and kicks every bad sector/friend found.
    */



    VXDTFRawSecMapTypedef::StrippedRawSecMap m_sectorMap; /**< contains full information of the sectorMap */
    std::string m_nameOfSecMap; /**< Name of the sectorMap */
    std::string m_detectorType; /**< Name of the detectorType (PXD, SVD, VXD) */
    std::vector<double> m_sectorConfigU; /**< allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    std::vector<double> m_sectorConfigV; /**< allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0 */
    TVector3 m_origin; /**< defines the position of the assumed primary vertex */
    double m_magneticFieldStrength; /**< strength of magnetic field in Tesla, standard is 1.5T */
    double m_lowPt; /**< lower threshold for transverse momentum stored in that map */
    double m_highPt; /**< set higher threshold for transverse momentum stored in that map */
    double m_minDistance2origin; /**< defines minimal accepted distance for sectors to the origin */
    double m_maxDistance2origin; /**< defines maximum accepted distance for sectors to the origin */
    bool m_sortByDistance2origin; /**< if true, filterBadSectorCombis filters sectors by origin not by layerID */

    std::pair<double, double> m_cutoffQuantiles; /**< .first is lower quantile for cutoff, .second is higher quantile */
    std::pair<double, double> m_smallCutoffQuantiles; /**< same as for 'm_cutoffQuantiles' but for sector-combinations with small sample size */
    double m_smallSampleThreshold; /**< defines threshold for big samples, if sampleSize is smaller than threshold, m_minSampleThresholds will be used leading to quantiles for smallCutoffSamples */
    double m_minSampleThreshold; /**< if sample size of current sector-combination is smaller than this value, it gets rejected */
    double m_smallStretchFactor; /**< is a stretchFactor by which the value will be stretched if the value is from a small sample. The value will be increased(for max cutoffs)/decreased(for min cutoffs) by m_smallStretchFactor times the original value -> if = 0 -> no stretching */
    double m_stretchFactor; /**< is a stretchFactor by which the value will be stretched if the value is from a normal sample. The value will be increased(for max cutoffs)/decreased(for min cutoffs) by m_stretchFactor times the original value  -> if = 0 -> no stretching */
    std::pair<bool, double> m_rareSectorCombinations; /**< .first allows check for rare sectorCombinations if true, .second is the rareness-threshold (examples: 1. = 100%, 0.001 = 0.1%) if a sector-friend-combination occurs less than threshold value compared to total occurrence of current sector, it gets deleted*/
    std::pair<bool, int> m_maxLayerLevelDifference; /**< .first allows check for maximum difference in level of layers for current sector-friend-combination, .second sets max level difference (example: .second is 2, layerID of sector is 6, of friend is 3 -> more than threshold -> friend gets kicked) missing layers by choice of detector type are considered */
    bool m_removeDeadSectorChains; /**< if true, not only sectors having no friends after first filtering iteration are deleted but also all combinations where dead sectors were friends. If these combinations can lead to kill more sectors and therefore the map gets holey but clean of non-existing cases */
    VXDTFRawSecMapTypedef::SectorDistancesMap m_dist2OriginMap; /**< stores the secID in .first and the value for the distances in .second */

    ClassDef(VXDTFRawSecMap, 4)
  };

  /** @}*/
} //Belle2 namespace
