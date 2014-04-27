/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FADC_APV_MAPPER_H_
#define FADC_APV_MAPPER_H_

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDDigit.h>
#include <boost/property_tree/ptree.hpp>
#include <unordered_map>


namespace Belle2 {
  /** This class implements the methods to map raw SVD hits to BASF2 SVD hits.
   * Raw SVD hits are identified by: FADC number, APV number, strip number
   * Basf2 SVD hits are identified by PXD Sensor-id, side, strip number
   * MODIFICATIONS 06/01/2014 by PKvasnick:
   * The code no longer relies on consecutive numbering of FADCs or APVs.
   */


  class SVDOnlineToOfflineMap {
  public:

    /** Class to hold FADC+APV25 numbers */
    class ChipID {
    public:
      /** Typedefs of the compound id type and chip number types */
      typedef unsigned short baseType;
      /** Type of chip numbers */
      typedef unsigned char chipNumberType;
      /** Constructor taking a compound id */
      ChipID(baseType id = 0) { m_id.id = id; }
      /** Constructor taking chip numbers */
      ChipID(chipNumberType FADC, chipNumberType APV25)
      { m_id.parts.FADC = FADC; m_id.parts.APV25 = APV25; }
      /** Copy ctor */
      ChipID(const ChipID& other): m_id(other.m_id) {}
      /** Assignment from same type */
      ChipID& operator=(ChipID other) { m_id.id = other.m_id.id; return *this; }
      /** Assignment from base type */
      ChipID& operator=(baseType id) { m_id.id = id; return *this; }
      /** cast to base type */
      operator baseType() { return m_id.id; }
      /** equality */
      bool operator==(const ChipID& other) const { return (m_id.id == other.m_id.id); }
      /** ordering */
      bool operator<(const ChipID& other) const { return (m_id.id < other.m_id.id); }
      /** Get chip ID */
      baseType getID() const { return m_id.id; }
      /** Get FADC number */
      chipNumberType getFADC() const {return m_id.parts.FADC; }
      /** Get APV25 number */
      chipNumberType getAPV25() const {return m_id.parts.APV25; }
      /** Set chip ID */
      void setID(baseType id) {  m_id.id = id; }
      /** Set FADC number */
      void setFADC(chipNumberType FADC) { m_id.parts.FADC = FADC; }
      /** Set APV25 number */
      void setAPV25(chipNumberType APV25) { m_id.parts.APV25 = APV25; }
    private:
      /** Union type representing the ChipID compound */
      union {
        /** unique id */
        baseType id : 8 * sizeof(baseType);
        /** Alternative struct representation */
        struct {
          chipNumberType FADC : 8 * sizeof(chipNumberType);
          chipNumberType APV25: 8 * sizeof(chipNumberType);
        } parts;
      } m_id;
    }; //ChipID class
    /** Struct to hold data about an APV25 chip.*/
    struct ChipInfo {
      VxdID m_sensorID;           /**< Sensor ID */
      bool m_uSide;               /**< True if u-side of the sensor */
      bool m_parallel;            /**< False if numbering is reversed */
      unsigned short m_channel0;   /**< Strip corresponding to channel 0 */
      unsigned short m_channel127; /**< Strip corresponding to channel 127 */
    }; // ChipInfo struct

    // SVDOnlineOffLineMap

    /** Constructor
     * @param xml_filename is the name of the xml file containing the map.
     */
    SVDOnlineToOfflineMap(const std::string& xml_filename);

    /** No default constructor */
    SVDOnlineToOfflineMap() = delete;

    /** Return a pointer to a new SVDDigit whose VxdID, isU and cellID is set
     * @param FADC is FADC number from the SVDRawCopper data.
     * @param APV25 is the APV25 number from the SVDRawCopper data.
     * @param channel is the APV25 channel number from the SVDRawCopper data.
     * @return a pointer to the new SVDDigit owned by the caller whose
     * Position is 0
     * FIXME: There should be no such function in this mapping class, no dependence
     * on SVDDigit and its interface.
     */
    SVDDigit* NewDigit(unsigned char FADC, unsigned char APV25,
                       unsigned char channel, float charge, float time);

    /** Get ChipInfo for a given FADC/APV combination.
     * @param FADC is FADC number from the SVDRawCopper data.
     * @param APV25 is the APV25 number from the SVDRawCopper data.
     * @return a reference to the corresponding ChipInfo object, all-zero if
     * nonsensical input.
     */
    const ChipInfo& getChipInfo(unsigned char FADC, unsigned char APV25);

    /** Convert APV channel number to a strip number using a ChipInfo object.
     * @param channel APV25 channel
     * @param info Const reference to ChipInfo object.
     * @return The corresponding strip number, -1 if nonsensical input
     */
    short getStripNumber(unsigned char channel, const ChipInfo& info) const
    { return (info.m_channel0 + ((unsigned short)channel) * (info.m_parallel ? 1 : -1)); }


  private:

    /** Read from the ptree v in the xml file the layer nLayer
     */
    void ReadLayer(int nLayer,  boost::property_tree::ptree const& xml_layer);

    /** Read from the ptree xml_ladde the ladder nLadder in layer nLayer
     */
    void ReadLadder(int nLayer, int nLadder, boost::property_tree::ptree const& xml_ladder);

    /** Read from the ptree xml_sensor the sensor nSensor in ladder nLadder in layer nLayer
     */
    void ReadSensor(int nLayer, int nLadder, int nSensor, boost::property_tree::ptree const& xml_sensor);

    /** Read from the ptree xml_side the U-side, if isU, (the V-side otherwise) of the sensor
     *  nSensor in ladder nLadder in layer nLayer
     */
    void ReadSensorSide(int nLayer, int nLadder, int nSensor, bool isU, boost::property_tree::ptree const& xml_side);

    /** m_chips[ChipID(FADC,APV25)] gives the ChipInfo for the given APV25 on
     * the given FADC.
     */
    std::unordered_map< ChipID::baseType, ChipInfo > m_chips;

    /** add chipN on FADCn to the map
     */
    void addChip(unsigned char  chipN,
                 unsigned char  FADCn,
                 int nlayer, int nladder, int nsensor, bool isU,
                 unsigned short stripNumberCh0,
                 bool           isParallel);

    void addChip(unsigned char  chipN,
                 unsigned char  FADCn,
                 unsigned short stripNumberCh0,
                 bool           isParallel
                );

    ChipInfo m_currentChipInfo; /**< internal instance of chipinfo used by the getter */

  };

} // namespace Belle2
#endif

