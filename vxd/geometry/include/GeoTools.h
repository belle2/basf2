/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace VXD {

    /** The class collects utility functions for numbering layers, sensors
     * snd chips based on current VXD geometry.
     */
    class GeoTools {

    public:
      /** Constructor builds lookup maps from GeoCache.
       * The constructor calls VXD::GeoCache, should be called in
       * initialize() method of modules, not in module constructor.
       */
      GeoTools();

      /*********************************************************************
       * General geometry parameters
       ********************************************************************/

      /** Get number of VXD layers
       * @return number of VXD layers in current geometry
       */
      unsigned short getNumberOfLayers() const { return m_listOfLayers.size(); }

      /** Get number of PXD layers
       * @return number of PXD layers in current geometry
       */
      unsigned short getNumberOfPXDLayers() const { return m_firstSVDLayer; }

      /** Get number of SVD layers
       * @return number of SVD layers in current geometry
       */
      unsigned short getNumberOfSVDLayers() const
      { return m_listOfLayers.size() - m_firstSVDLayer; }

      /** Get numbers of VXD layers
       * @return std::vector with numbers of VXD layers
       */
      std::vector<unsigned short> getLayers() const { return m_listOfLayers; }

      /** Get numbers of PXD layers
       * @return std::vector with numbers of PXD layers
       */
      std::vector<unsigned short> getPXDLayers() const
      {
        std::vector<unsigned short> pxdLayers;
        std::copy_if(m_listOfLayers.begin(), m_listOfLayers.end(),
                     std::back_inserter(pxdLayers),
                     [](unsigned short l)->bool {return l < 3;});
        return pxdLayers;
      }

      /** Get numbers of SVD layers
       * @return std::vector with numbers of SVD layers
       */
      std::vector<unsigned short> getSVDLayers() const
      {
        std::vector<unsigned short> svdLayers;
        std::copy_if(m_listOfLayers.begin(), m_listOfLayers.end(),
                     std::back_inserter(svdLayers),
                     [](unsigned short l)->bool {return l > 2;});
        return svdLayers;
      }

      /** Get first (innermost) layer number
       * @return number of innermost layer
       */
      unsigned short getFirstLayer() const { return *m_listOfLayers.begin(); }

      /** Get last (outermost) layer number
       * @return number of last (outermost) layer
       */
      unsigned short getLastLayer() const { return *m_listOfLayers.rbegin(); }

      /** Get first (innermost) PXD layer number
       * @return number of innermost PXD layer, -1 if no PXD layers
       */
      short getFirstPXDLayer() const
      {
        return (m_firstSVDLayer > 0 ? *m_listOfLayers.begin() : -1);
      }

      /** Get last (outermost) PXD layer number
       * @return number of last (outermost) layer -1 if no PXD layers
       */
      short getLastPXDLayer() const
      {
        return (m_firstSVDLayer > 0 ? m_listOfLayers[m_firstSVDLayer - 1] : -1);
      }

      /** Get first (innermost) SVD layer number
       * @return number of innermost SVD layer, -1 if no SVD layers
       */
      short getFirstSVDLayer() const
      {
        return (m_firstSVDLayer < m_listOfLayers.size() ? m_listOfLayers[m_firstSVDLayer] : -1);
      }

      /** Get last (outermost) SVD layer number
       * @return number of last (outermost) layer -1 if no SVD layers
       */
      short getLastSVDLayer() const
      {
        return (m_firstSVDLayer < m_listOfLayers.size() ? *m_listOfLayers.rbegin() : -1);
      }

      /** Get total number of sensors
       * @return total number of sensors, PXD+SVD
       */
      unsigned short getNumberOfSensors() const { return m_listOfSensors.size(); }

      /** Get number of PXD sensors
       * @return number of PXD sensors
       */
      unsigned short getNumberOfPXDSensors() const { return m_firstSVDIndex; }

      /** Get number of SVD sensors
       * @return number of SVD sensors
       */
      unsigned short getNumberOfSVDSensors() const
      { return m_listOfSensors.size() - m_firstSVDIndex; }

      /*********************************************************************
       * Chip-related parameters
       *********************************************************************/
      /** Get total number of chips in PXD
       * @return total number of chips in PXD
       */
      size_t getTotalPXDChips() const { return m_listOfPXDChips.size(); }

      /** Get number of u-side PXD chips
       * @return number of u-side chips
       */
      unsigned short getNumberOfPXDUSideChips() const
      { return c_nPXDChipsU; }

      /** Get number of v-side PXD chips
       * @return number of v-side chips
       */
      unsigned short getNumberOfPXDVSideChips() const
      { return c_nPXDChipsV; }

      /** Get number of PXD readout gates
       * @return number of v-side PXD readout gates
       */
      unsigned short getNumberOfPXDReadoutGates() const
      { return c_nPXDReadoutGates; }

      /** Get total number of chips in SVD
       * @return total number of chips in SVD
       */
      size_t getTotalSVDChips() const { return m_listOfSVDChips.size(); }

      /** Get number of u-side SVD chips
       * @return number of u-side chips
       */
      unsigned short getNumberOfSVDUSideChips() const
      { return c_nSVDChipsLu; }

      /** Get number of v-side SVD chips
       * @param layer Layer of the sensor
       * @return number of v-side chips
       */
      unsigned short getNumberOfSVDVSideChips(unsigned short layer) const
      { return (layer == 3 ? c_nSVDChipsL3 : c_nSVDChipsLv); }

      /** Get number of strips per APV chip in SVD
       * @return number of chips per APV in SVD.
       */
      unsigned short getSVDChannelsPerChip() const
      { return c_nSVDChannelsPerChip; }

      /*********************************************************************
       * General sensor indexing
       *********************************************************************/
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

      /** Reverse lookup VxdID from list index.
       * @param vxdIndex list index of sensor in geometry (PXD then SVD)
       * @return VxdID of the sensor
       * NB: Use specialized moethod to get VxdIDs for SVD-only indices!
       */
      VxdID getSensorIDFromIndex(int vxdIndex) const
      {
        return m_listOfSensors[vxdIndex];
      }

      /*********************************************************************
       * PXD sensor indexing
       *********************************************************************/
      /** Return index of sensor in plots.
       * A simple alias to getSensorIndex()
       * @param sensorID VxdID of the sensor
       * @return Index of sensor in plots.
       */
      int getPXDSensorIndex(VxdID sensorID) const
      {
        return getSensorIndex(sensorID);
      }
      /** Return index of sensor in plots.
       * Use for only PXD sensor counting, without SVD.
       * @param layer layer position of sensor
       * @param ladder ladder position of sensor
       * @param sensor sensor position of sensor
       * @return Index of sensor in plots.
       */
      int getPXDSensorIndex(int layer, int ladder, int sensor) const
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

      /*********************************************************************
       * PXD chip indexing
       ********************************************************************/
      /** Return PXD chip index in the list of PXD chips.
        * 1-based chip numbering.
        * @param sensorID VxdID of the sensor
      * @param isU true for u side of the sensor (DCD)
        * @param chip Chip position on sensor - DCDs or Switchers
        * @return Index of sensor in plots.
        */
      int getPXDChipIndex(VxdID sensorID, bool isU, int chip) const
      {
        VxdID chipID(sensorID);
        chip = isU ? chip - 1 : chip - 1 + c_nPXDChipsU;
        chipID.setSegmentNumber(static_cast<unsigned short>(chip));
        return std::distance(
                 m_listOfPXDChips.begin(),
                 std::find(m_listOfPXDChips.begin(), m_listOfPXDChips.end(), chipID));
      }

      /** Return PXD chip index in the list of PXD chips.
       * @param layer layer position of sensor
       * @param ladder ladder position of sensor
       * @param sensor sensor position of sensor
       * @param isU true for u side of the sensor (DCD)
       * @param chip Chip number on sensor (DCD or switcher, 1-based)
       * @return Index of sensor in plots.
       */
      int getPXDChipIndex(int layer, int ladder, int sensor, bool isU, int chip) const
      {
        return getPXDChipIndex(VxdID(layer, ladder, sensor), isU, chip);
      }

      /** Return chipID (VxdID + side and chipNo) for index in the list.
       * @param pxdChipIndex Index in the list of PXD sensors.
       * @return chipID: VxdID + chip number/side encoded in segment part.
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

      /** Decode (1-based) chip  number from a PXD ChipID.
       * @param chipID VxdID with chip number encoded in Segment part.
       * @return Chip number on the respective sensor side.
       */
      unsigned short getPXDChipNumber(VxdID chipID) const
      {
        unsigned short chipNo = chipID.getSegmentNumber();
        return (chipNo < c_nPXDChipsU ? chipNo + 1 : (chipNo + 1 - c_nPXDChipsU));
      }

      /*********************************************************************
       * SVD sensor indexing
       *********************************************************************/
      /** Return index of SVD sensor in plots.
       * This is solely SVD counting starting from 0.
       * @param sensorID VxdID of the sensor
       * @return Index of sensor in plots.
       */
      int getSVDSensorIndex(VxdID sensorID) const
      {
        return (getSensorIndex(sensorID) - m_firstSVDIndex);
      }

      /** Return index of SVD sensor in plots.
       * This is solely SVD counting starting from 0.
       * @param layer layer position of sensor
       * @param ladder ladder position of sensor
       * @param sensor sensor position of sensor
        * @return Index of sensor in plots.
        */
      int getSVDSensorIndex(int layer, int ladder, int sensor) const
      {
        return getSVDSensorIndex(VxdID(layer, ladder, sensor));
      }

      /** Return VxdID for SVD index of sensor in plots.
       * Uses SVD sensor count, without PXD.
       * @param svdIndex Index of the SVD sensor.
       */
      VxdID getSensorIDFromSVDIndex(int svdIndex) const
      {
        return m_listOfSensors[m_firstSVDIndex + svdIndex];
      }

      /*********************************************************************
       * SVD chip indexing
       ********************************************************************/

      /** Return SVD chip index in the list of SVD chips.
       * @param sensorID VxdID of the sensor
       * @param isU true for u side of the sensor
       * @param chip Chip position on sensor (1-based)
       * @return Index of sensor in plots.
       */
      int getSVDChipIndex(VxdID sensorID, bool isU, int chip) const
      {
        VxdID chipID(sensorID);
        if (sensorID.getLayerNumber() == 3)
          chip = isU ? chip - 1 : chip - 1 + c_nSVDChipsL3;
        else
          chip = isU ? chip - 1 : chip - 1 + c_nSVDChipsLu;
        chipID.setSegmentNumber(static_cast<unsigned short>(chip));
        return std::distance(
                 m_listOfSVDChips.begin(),
                 std::find(m_listOfSVDChips.begin(), m_listOfSVDChips.end(), chipID));
      }

      /** Return SVD chip index in the list of SVD chips.
       * @param layer layer position of sensor
       * @param ladder ladder position of sensor
       * @param sensor sensor position of sensor
       * @param isU true for u side of the sensor
       * @param chip Chip position on sensor (1-based)
       * @return Index of sensor in plots.
       */
      int getSVDChipIndex(int layer, int ladder, int sensor, bool isU, int chip) const
      {
        return getSVDChipIndex(VxdID(layer, ladder, sensor), isU, chip);
      }

      /** Return chipID (VxdID with side and chipNo) for index in the list.
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
        /* cppcheck-suppress duplicateExpressionTernary */
        return (chipID.getLayerNumber() == 3 ? chipID.getSegmentNumber() < c_nSVDChipsL3 : chipID.getSegmentNumber() < c_nSVDChipsLu);
      }

      /** Decode (1-based) chip number from a SVD ChipID.
       * @param chipID VxdID with chip number encoded in Segment part.
       * @return Chip number on the respective sensor side.
       */
      unsigned short getSVDChipNumber(VxdID chipID) const
      {
        unsigned short chipNo = chipID.getSegmentNumber();
        if (chipID.getLayerNumber() == 3) {
          if (chipNo < c_nSVDChipsL3)
            return chipNo + 1;
          else
            return (chipNo + 1 - c_nSVDChipsL3);
        } else {
          if (chipNo < c_nSVDChipsLu)
            return chipNo + 1;
          else
            return (chipNo + 1 - c_nSVDChipsLu);
        }
      }

      /*********************************************************************
       * Layer indexing
       ********************************************************************/
      /** Return index of layer in plots.
       * @param layer layer number.
       * @return Index of layer in plots.
       */
      int getLayerIndex(unsigned short layer) const
      {
        return std::distance(
                 m_listOfLayers.begin(),
                 std::find(m_listOfLayers.begin(), m_listOfLayers.end(), layer)
               );
      }

      /** Return layer number for list index
       * @param index List index of the layer
       * @return Layer number
       */
      unsigned short getLayerNumberFromLayerIndex(int index) const
      {
        return m_listOfLayers[index];
      }


    private:

      /** Create list of VXD layers */
      void createListOfLayers();

      /** Create list of PXD chips. */
      void createListOfPXDChips();

      /** Create list of SVD chips. */
      void createListOfSVDChips();

      /** List of all VXD sesnros */
      std::vector<VxdID> m_listOfSensors;

      /** Number of the first SVD sensor in the list */
      size_t m_firstSVDIndex;

      /** List of all VXD layers */
      std::vector<unsigned short> m_listOfLayers;

      /** List index of the first SVD layer */
      unsigned short m_firstSVDLayer;

      /** List of all PXD chips */
      std::vector<VxdID> m_listOfPXDChips;

      /** List of all SVD chips */
      std::vector<VxdID> m_listOfSVDChips;

      /** Number of PXD chips per sensor in u (DCD) (=4) on Belle II */
      const unsigned short c_nPXDChipsU = 4;
      /** Number of PXD chips per sensor in v (Switchers) (=6) on Belle II */
      const unsigned short c_nPXDChipsV = 6;
      /** Number of PXD readout gates (or total number of Switcher channels) on Belle II */
      const unsigned short c_nPXDReadoutGates = 192;
      /** Number of SVD chips per sensor in u,v in layer 3 (=6) on Belle II */
      const unsigned short c_nSVDChipsL3 = 6;
      /** Number of SVD chips per sensor in u in layers 4,5,6 (=6) on Belle II */
      const unsigned short c_nSVDChipsLu = 6;
      /** Number of SVD chips per sensor in v in layers 4,5,6 (=4) on Belle II */
      const unsigned short c_nSVDChipsLv = 4;
      /** Number of SVD strips per chip on Belle II */
      const unsigned short c_nSVDChannelsPerChip = 128;
    };
  } // VXD namespace
} // Belle2 namespace
