/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <string>
#include <ostream>



namespace Belle2 {

  /** Class to identify a sector inside of the VXD.
  *
  * It allows to uniquely identify a SecID within the Vxd-SectorMap:
  * The following information is coded into the FullSecID:
  *  Layer: bits 0 -  3 (Values 0-15, calculated using shift: 28)
  *  SubLayer:      bits 4 (Values 0,1, calculated using: 27)
  *  VxdID:       bits 5 - 21 (Values: whole range of VxdIDs, calculated using: 11)
  *  SecID:        bits 22 - 32 ( Values: 0 up to 2047)
  */
  class FullSecID {
  public:


    /** defines the base type used for FullSecID */
    typedef unsigned int BaseType;


    // starting with operator overloading...
    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const FullSecID& secID)  const { return m_fullSecID < secID.m_fullSecID; }


    /** overloaded '=='-operator for sorting algorithms */
    bool operator==(const FullSecID& secID) const { return m_fullSecID == secID.m_fullSecID; }

    /** comparison which ignores the sublayer ID */
    bool equalIgnoreSubLayerID(const FullSecID& secID)
    {
      return ((~FullSecID::SubLayerMask)&m_fullSecID) == ((~FullSecID::SubLayerMask)&secID.m_fullSecID);
    }

    /** overloaded assignment operator */
    FullSecID& operator=(const FullSecID& secID) { m_fullSecID = secID.m_fullSecID; return *this; }


    /** overloaded int cast */
    operator unsigned int() const { return m_fullSecID; }


    /** overloaded string cast */
    operator std::string(void) const { return getFullSecString(); }


    /** overloaded '<<' stream operator. Print secID to stream by converting it to string */
    friend std::ostream& operator<< (std::ostream& out, const FullSecID& secID) { out << secID.getFullSecString(); return out; }


    // constructors...
    /** Copy constructor. */
    FullSecID(const FullSecID& secID): m_fullSecID(secID.m_fullSecID) {}


    /** Constructor taking the encoded full Sec ID (int).
    * use this one if you have got an already existing integer storing the FullSecID information and you want the possibility to extract its entries (layer, sublayer, vxdID, secID)
    *
    *  @param id  Number of the SecID using the encoded format. Careful - SecIDs are not continuously within the encoded number.
    *                The default is set to zero which represents the secID of the IP
    */
    explicit FullSecID(unsigned int id = 0):
      m_fullSecID(id) {}


    /** Constructor taking the encoded full Sec ID (string).
    * use this one if you want to convert a string-encoded sector into the internal version (e.g. provided by the xml-files storing a sectormap) into the internal version. This constructor should be dispensable after changing from xml-based data storage to sql-storage
    *
    *  @param sid  global secID encoded in a string file eg "41_12345_3" 41 means layer 4, sublayer 1, 12345 is a dummy for the integer-version of the VxdID and the last digit is the local secID on the sensor
    */
    explicit FullSecID(std::string sid);


    /** Constructor. If you want to create a new Full Sec ID using the information which shall be stored there
    * (e.g. layer, sublayer, vxdID, secID)
    *
    * @param vxdID  full VxdID of sensor of current sector.
    * @param subLayerID   0 if not in overlapping area of sensor, 1 if within overlap.
    * @param sectorNumber    number of sector at current sensor/vxdID.
    */
    explicit FullSecID(VxdID vxdID, bool subLayerID = false, unsigned int sectorNumber = 0);


    /** Another contstructor which does not need a VxdID nor another coded valid FullSecID
    * @param layerID  set here the layer number as an ID...
    * @param subLayerID   0 if not in overlapping area of sensor, 1 if within overlap.
    * @param sensorID  an ID representing the sensorID (e.g. uniID/vxdID).
    * @param sectorNumber    number of sector at current sensor/vxdID.
    */
    FullSecID(unsigned int layerID, bool subLayerID, unsigned int sensorID, unsigned int sectorNumber);


    /** Destructor. */
    ~FullSecID() {}


    /** returns LayerID compatible with basf2 standards. */
    short int getLayerID() const { return getLayerNumber(); }


    /** returns LayerID compatible with basf2 standards. */
    short int getLayerNumber() const { return m_fullSecID >> LayerBitShift; }


    /** returns LadderID compatible with basf2 standards */
    int getLadderID() const { return getLadderNumber(); }


    /** returns LadderID compatible with basf2 standards */
    int getLadderNumber() const { return getVxdID().getLadderNumber(); }


    /** returns SubLayerID which tells you whether it is useful to search for compatible sectors in the same layer as well. */
    bool getSubLayerID() const { return (m_fullSecID bitand SubLayerMask) >> SubLayerBitShift; }


    /** returns VxdID of sensor. */
    VxdID getVxdID() const { return VxdID((m_fullSecID bitand VxdIDMask) >> VxdIDBitShift); }


    /** returns uniID of sensor (basically the same as VxdID (can simply converted to vxdID(uniID)) but a trifle faster when only the short int value storing the VxdID information is wanted). */
    unsigned short int getUniID() const { return (m_fullSecID bitand VxdIDMask) >> VxdIDBitShift; }


    /** returns SecID of current FullSecID (only unique for each sensor). */
    short int getSecID() const { return (m_fullSecID bitand SectorMask); }


    /** returns the FullSecID coded as integer for further use (can be reconverted to FullSecID by using FullSecID(value)) */
    unsigned int getFullSecID() const { return m_fullSecID; }


    /** returns the FullSecID coded as string compatible to secIDs stored in the xml-sectormaps */
    std::string getFullSecString() const;

  protected:
    /** Number of bits available to represent a layer */
    const static int LayerBits;


    /** Number of bits available to represent a subLayer (a sublayer is 1 if current sector on sensor is in an overlapping region, where it is possible that an inner hit of the same track could be on the same layer) */
    const static int SubLayerBits;


    /** Number of bits available to store a full vxdID */
    const static int VxdIDBits;


    /** Number of bits available to represent a sector */
    const static int SectorBits;


    /** Total bit size of the VxdID */
    const static int Bits;


    /** Maximum valid Layer ID */
    const static int MaxLayer;


    /** Maximum valid Ladder ID */
    const static int MaxSubLayer;


    /** Maximum valid Sensor ID */
    const static int MaxVxdID;


    /** Maximum valid Segment ID */
    const static int MaxSector;


    /** Maximum value for ID */
    const static int MaxID;


    /** Number of bits to shift for storing a LayerID */
    const static int LayerBitShift;


    /** Number of bits to shift for storing a SubLayerID */
    const static int SubLayerBitShift;


    /** Number of bits shift for storing a full vxdID */
    const static int VxdIDBitShift;


    /** mask to get subLayerID from fullSecID */
    const static int SubLayerMask;


    /** mask to get VxdID from fullSecID */
    const static int VxdIDMask;


    /** mask to get SectorID from fullSecID */
    const static int SectorMask;

    /** contains full info of current SecID. Check class definition for detailed description */
    BaseType m_fullSecID;

  }; //end class SecID
} //end namespace Belle2
