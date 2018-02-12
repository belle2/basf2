/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "TH1F.h"
#include "TVectorT.h"
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>
#include <set>
#include <memory>
#include <algorithm>
#include <numeric>

namespace Belle2 {

  /** The class collects utility functions for creating DQM histograms:
    * prepared for PXD, SVD, VXD and Track DQM histograms.
    */
  class DQMCommonUtils {

  public:
    /** Constructor builds lookup maps from GeoCache.
     * The constructor calls VXD::GeoCache, should be called in
     * initialize() method of modules, not in module constructor.
     */
    DQMCommonUtils()
    {
      VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      m_listOfSensors = geo.getListOfSensors();
      m_firstSVDIndex = std::distance(
                          m_listOfSensors.begin(),
                          std::find_if(m_listOfSensors.begin(), m_listOfSensors.end(),
                                       [](VxdID id)->bool { return (id.getLayerNumber() > 2); })
                        ); // works correctly even with no SVD sensors
      auto layerSet = geo.getLayers();
      std::transform(
        layerSet.begin(), layerSet.end(),
        std::back_inserter(m_listOfLayers),
        [](VxdID id)->int { return id.getLayerNumber(); }
      );
      createListOfPXDChips();
      createListOfSVDChips();
    }

    /** Return index of s VXD sensor for plotting.
     * @param sensorID VxdID of the sensor
     * @return Index of sensor in plots.
     */
    int getSensorIndex(VxdID sensorID) const
    {
      return std::distance(
               m_listOfSensors.begin(),
               find(m_listOfSensors.begin(), m_listOfSensors.end(), sensorID)
             );
    }

    /** Function return index of sensor in plots.
      * Use for only PXD sensor counting, without SVD.
      * @param layer layer position of sensor
      * @param ladder ladder position of sensor
      * @param sensor sensor position of sensor
      * @return Index of sensor in plots.
      */
    int getPXDsensorIndex(int layer, int ladder, int sensor) const
    {
      return getSensorIndex(VxdID(layer, ladder, sensor));
    }

    /** Return index of a PXD sensor in plots.
      * Use for only PXD sensor counting, without SVD.
      * @param pxdIndex Index of sensor in plots.
      */
    VxdID getSensorIDFromPXDIndex(int pxdIndex) const
    {
      return m_listOfSensors[pxdIndex];
    }

    /** Return PXD chip index in the list of PXD chips.
      * @param layer layer position of sensor
      * @param ladder ladder position of sensor
      * @param sensor sensor position of sensor
      * @param chip Chip position on sensor - DCDs or Switchers
      * @param isU true for u side of the sensor (DCD)
      * @return Index of sensor in plots.
      */
    int getPXDChipIndex(int layer, int ladder, int sensor, int chip, int isU) const
    {
      VxdID chipID(layer, ladder, sensor);
      chip = isU ? chip : chip + c_nPXDChipsU;
      chipID.setSegmentNumber(static_cast<unsigned short>(chip));
      return std::distance(
               m_listOfPXDChips.begin(),
               std::find(m_listOfPXDChips.begin(), m_listOfPXDChips.end(), chipID));
    }

    /** Return position of a chip on a PXD sensor for index in the list. .
      * @param pxdChipIndex Index in the list of PXD sensors.
      * @return chipID: VxdID with chip number/side encoded in segment part.
      */
    VxdID getChipIDFromPXDIndex(int pxdChipIndex) const
    {
      return m_listOfPXDChips[pxdChipIndex];
    }

    /** Decode sensor side from a PXD ChipID.
     * @param chipID VxdID with chip number encoded in Segment part.
     * @return true if the chip is on u-side.
     */
    bool isPXDSideU(VxdID chipID) const
    {
      return (chipID.getSegmentNumber() < c_nPXDChipsU);
    }

    /** Decode sensor number from a PXD ChipID.
     * @param chipID VxdID with chip number encoded in Segment part.
     * @return Chip number on the respective sensor side.
     */
    unsigned short getPXDChipNumber(VxdID chipID) const
    {
      unsigned short chipNo = chipID.getSegmentNumber();
      return (chipNo < c_nPXDChipsU ? chipNo : (chipNo - c_nPXDChipsU));
    }

    /** Function return index of SVD sensor in plots.
      * Use for only SVD sensor counting, without PXD.
      * @param layer layer position of sensor
      * @param ladder ladder position of sensor
      * @param sensor sensor position of sensor
      * @return Index of sensor in plots.
      */
    int getSVDSensorIndex(int layer, int ladder, int sensor) const
    {
      return (getSensorIndex(VxdID(layer, ladder, sensor)) - m_firstSVDIndex);
    }

    /** Function return VxdID for SVD index of sensor in plots.
      * Uses SVD sensor count, without PXD.
      * @param svdIndex Index of the SVD sensor.
      */
    VxdID getSensorIDFromSVDIndex(int svdIndex) const
    {
      return m_listOfSensors[m_firstSVDIndex + svdIndex];
    }

    /** Return SVD chip index in the list of SVD chips.
      * @param layer layer position of sensor
      * @param ladder ladder position of sensor
      * @param sensor sensor position of sensor
      * @param chip Chip position on sensor
      * @param isU true for u side of the sensor
      * @return Index of sensor in plots.
      */
    int getSVDChipIndex(int layer, int ladder, int sensor, int chip, int isU) const
    {
      VxdID chipID(layer, ladder, sensor);
      if (layer == 3)
        chip = isU ? chip : chip + c_nSVDChipsL3;
      else
        chip = isU ? chip : chip + c_nSVDChipsLu;
      chipID.setSegmentNumber(static_cast<unsigned short>(chip));
      return std::distance(
               m_listOfSVDChips.begin(),
               std::find(m_listOfSVDChips.begin(), m_listOfSVDChips.end(), chipID));
    }

    /** Return position of a chip on a SVD sensor for index in the list. .
      * @param svdChipIndex Index in the list of SVD sensors.
      * @return chipID: VxdID with chip number/side encoded in segment part.
      */
    VxdID getChipIDFromSVDIndex(int svdChipIndex) const
    {
      return m_listOfSVDChips[svdChipIndex];
    }

    /** Decode sensor side from a SVD ChipID.
     * @param chipID VxdID with chip number encoded in Segment part.
     * @return true if the chip is on u-side.
     */
    bool isSVDSideU(VxdID chipID) const
    {
      return (chipID.getLayerNumber() == 3 ? chipID.getSegmentNumber() < c_nSVDChipsL3 : chipID.getSegmentNumber() < c_nSVDChipsLu);
    }

    /** Decode sensor number from a SVD ChipID.
     * @param chipID VxdID with chip number encoded in Segment part.
     * @return Chip number on the respective sensor side.
     */
    unsigned short getSVDChipNumber(VxdID chipID) const
    {
      unsigned short chipNo = chipID.getSegmentNumber();
      if (chipID.getLayerNumber() == 3) {
        if (chipNo < c_nSVDChipsL3)
          return chipNo;
        else
          return (chipNo - c_nSVDChipsL3);
      } else {
        if (chipNo < c_nSVDChipsLu)
          return chipNo;
        else
          return (chipNo - c_nSVDChipsLu);
      }
    }

    /** Return index of layer in plots.
      * Use for only PXD+SVD sensor counting.
      * @param layer layer position.
      * @return Index of layer in plots.
      */
    int getVXDLayerIndex(unsigned short layer) const
    {
      return std::distance(
               m_listOfLayers.begin(),
               std::find(m_listOfLayers.begin(), m_listOfLayers.end(), layer)
             );
    }
    /** Function return index of layer in plots.
      * Use for only PXD+SVD sensor counting.
      * @param index Index of layer in plots.
      * @return Layer number
      */
    unsigned short getLayerNumberFromVXDLayerIndex(int index) const
    {
      return m_listOfLayers[index];
    }
    /** Function return index of sensor in plots.
      * Use for only PXD+SVD sensor counting.
      * @param layer layer position of sensor.
      * @param ladder ladder position of sensor.
      * @param sensor sensor position of sensor.
      * @return Index of sensor in plots.
      */
    int getVXDSensorIndex(int layer, int ladder, int sensor) const
    {
      return getSensorIndex(VxdID(layer, ladder, sensor));
    }

    /** Reverse lookup VxdID from list index.
     * @param vxdIndex array index of sensor in geometry (PXD then SVD)
     * @return VxdID of the sensor
     * NB: Use specialized moethod to get VxdIDs for SVD-only indices!
     */
    VxdID getSensorIDFromIndex(int vxdIndex) const
    {
      return m_listOfSensors[vxdIndex];
    }

    /** Function return flag histogram filled based on condition from TH1F source.
      * Flag values:
      * -3: nonexisting Type
      * -2: histogram is missing or masked
      * -1: less than 100 samles, skip comparition
      *  0: good much with reference
      *  1: warning level = diff > 6 * sigma and < error level
      *  2: error level = diff > 10 * sigma
      * @param Type Set type of condition for flag calculation.
      * 1: use counts, mean and RMS.
      * 2: use counts only.
      * 3: use mean only.
      * 4: use RMS only.
      * 5: use counts and mean.
      * 9: use bin content only.
      * 10: use Chi2 condition and pars[0] and pars[1].
      * 100: nothing do just fill flags as OK.
      * @param bin bin which is fill in flag histogram.
      * @param pars array of parameters need for condition.
      * @param ratio Ratio of acquired events to reference events.
      * @param hist Histogram of sources.
      * @param refhist Reference histogram.
      * @param flag Histogram of flags.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int SetFlag(int Type, int bin, const double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist) const;
    /** Function return flag histogram filled based on condition from TH1I source.
      * Flag values:
      * -3: nonexisting Type
      * -2: histogram is missing or masked
      * -1: less than 100 samles, skip comparition
      *  0: good much with reference
      *  1: warning level = diff > 6 * sigma and < error level
      *  2: error level = diff > 10 * sigma
      * @param Type Set type of condition for flag calculation.
      * 1: use counts, mean and RMS.
      * 2: use counts only.
      * 3: use mean only.
      * 4: use RMS only.
      * 5: use counts and mean.
      * 9: use bin content only.
      * 10: use Chi2 condition and pars[0] and pars[1].
      * 100: nothing do just fill flags as OK.
      * @param bin bin which is fill in flag histogram.
      * @param pars array of parameters need for condition.
      * @param ratio Ratio of acquired events to reference events.
      * @param hist Histogram of sources.
      * @param refhist Reference histogram.
      * @param flag Histogram of flags.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int SetFlag2(int Type, int bin, const double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist) const;
    /** Function return flag histogram filled based on condition from TH1F source.
        * Flag values:
        * -3: nonexisting Type
        * -2: histogram is missing or masked
        * -1: less than 100 samles, skip comparition
        *  0: good much with reference
        *  1: warning level = diff > 6 * sigma and < error level
        *  2: error level = diff > 10 * sigma
        * @param Type Set type of condition for flag calculation.
        * 1: use counts, mean and RMS.
        * 2: use counts only.
        * 3: use mean only.
        * 4: use RMS only.
        * 5: use counts and mean.
        * 9: use bin content only.
        * 10: use Chi2 condition and pars[0] and pars[1].
        * 100: nothing do just fill flags as OK.
        * @param bin bin which is fill in flag histogram.
        * @param pars array of parameters need for condition.
        * @param ratio Ratio of acquired events to reference events.
        * @param hist Histogram of sources.
        * @param refhist Reference histogram.
        * @param flag Histogram of flags.
        * @return Indication of succes of realizing of condition, 1: OK.
        */
    // int SetFlag(const int Type, const int bin, double* pars, const double ratio, std::string name_hist, std::string name_refhist, TH1I* flaghist) const;
    //TH1* findHistLocal(TString& a);
    // TH1* GetHisto(TString histoname) const;

    /** Function for filling of TH1F histogram to database.
      * @param HistoBD Histogram for DB.
      */
    void CreateDBHisto(TH1F* HistoBD) const;
    /** Function for filling of TH1I histogram to database.
      * @param HistoBD Histogram for DB.
      */
    void CreateDBHisto(TH1I* HistoBD) const;

    /** Function for filling of group of TH1F histogram to database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to glue to one.
      */
    void CreateDBHistoGroup(TH1F** HistoBD, int number) const;
    /** Function for filling of group of TH1I histogram to database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to glue to one.
      */
    void CreateDBHistoGroup(TH1I** HistoBD, int number) const;

    /** Function for loading of TH1F histogram from database.
      * @param HistoBD Histogram for DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHisto(TH1F* HistoBD) const;
    /** Function for loading of TH1I histogram from database.
      * @param HistoBD Histogram for DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHisto(TH1I* HistoBD) const;

    /** Function for loading of group of TH1F histogram from database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to extract from DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHistoGroup(TH1F** HistoBD, int number) const;
    /** Function for loading of group of TH1I histogram from database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to extract from DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHistoGroup(TH1I** HistoBD, int number) const;


    /** Number of SVD strips per chip on Belle II */
    const int c_nSVDChannelsPerChip = 128;

  private:

    /** Create list of PXD chips. */
    void createListOfPXDChips()
    {
      // We set segment number to the number of chip.
      std::vector<unsigned short> sensorChips(c_nPXDChipsU + c_nPXDChipsV);
      std::iota(sensorChips.begin(), sensorChips.end(), 0);
      for (auto sensorID : m_listOfSensors) {
        std::vector<VxdID> sensorIDs;
        std::transform(
          sensorChips.begin(), sensorChips.end(),
          sensorIDs.begin(),
          [sensorID](unsigned short i)->VxdID
        { VxdID sID(sensorID); sID.setSegmentNumber(i); return sID; }
        );
        m_listOfPXDChips.insert(m_listOfPXDChips.end(), sensorIDs.begin(), sensorIDs.end());
      }
    }


    /** Create list of SVD chips. */
    void createListOfSVDChips()
    {
      // We set segment number to the number of chip.
      std::vector<unsigned short> sensorChipsL456(c_nSVDChipsLu + c_nSVDChipsLv);
      std::iota(sensorChipsL456.begin(), sensorChipsL456.end(), 0);
      std::vector<unsigned short> sensorChipsL3(2 * c_nSVDChipsL3);
      std::iota(sensorChipsL3.begin(), sensorChipsL3.end(), 0);
      for (auto sensorID : m_listOfSensors) {
        std::vector<VxdID> sensorIDs;
        std::vector<unsigned short>& sensorChips =
          sensorID.getLayerNumber() == 3 ? sensorChipsL3 : sensorChipsL456;
        std::transform(
          sensorChips.begin(), sensorChips.end(),
          sensorIDs.begin(),
          [sensorID](unsigned short i)->VxdID
        { VxdID sID(sensorID); sID.setSegmentNumber(i); return sID; }
        );
        m_listOfSVDChips.insert(m_listOfSVDChips.end(), sensorIDs.begin(), sensorIDs.end());
      }
    }

    /** List of all VXD sesnros */
    std::vector<VxdID> m_listOfSensors;

    /** Number of the first SVD sensor in the list */
    size_t m_firstSVDIndex;

    /** List of all VXD layers */
    std::vector<unsigned short> m_listOfLayers;

    /** List of all PXD chips */
    std::vector<VxdID> m_listOfPXDChips;

    /** List of all SVD chips */
    std::vector<VxdID> m_listOfSVDChips;

    /** Number of PXD chips per sensor in u (DCD) (=4) on Belle II */
    const int c_nPXDChipsU = 4;
    /** Number of PXD chips per sensor in v (Switchers) (=6) on Belle II */
    const int c_nPXDChipsV = 6;
    /** Number of SVD chips per sensor in u,v in layer 3 (=6) on Belle II */
    const int c_nSVDChipsL3 = 6;
    /** Number of SVD chips per sensor in u in layers 4,5,6 (=6) on Belle II */
    const int c_nSVDChipsLu = 6;
    /** Number of SVD chips per sensor in v in layers 4,5,6 (=4) on Belle II */
    const int c_nSVDChipsLv = 4;

  };

}
