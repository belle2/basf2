/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef FullSecID_H
#define FullSecID_H

#include <vxd/dataobjects/VxdID.h>
#include <string>



namespace Belle2 {
  namespace Tracking {

    /** Class to identify a sector inside of the VXD.
    *
    * It allows to uniquely identify a SecID within the Vxd-SectorMap:
    *  There are three representations for this: <br>
    *  - Super-Layer, Layer within the Super-Layer, Wire within the Layer as documented
    *  <a href="http://ekpbelle2.physik.uni-karlsruhe.de/~twiki/pub/Detector/CDC/WebHome/cdc_cell_num.pdf">here</a>, <br>
    *  - Layer as continuous counted through all Super-Layers, Wire within the Layer, <br>
    *  - Wire number encoded into a single unsigned short. This works as following: <br>
    *  Layer: bits 0 -  3 (Values 0-6, calculated using shift: 28)           <br>
    *  SubLayer:      bits 4 (Values 0,1, calculated using: 27)   <br>
    *  VxdID:       bits 5 - 21 (Values: whole range of VxdIDs, calculated using: 11)           <br>
    *  SecID:        bits 22 - 32 ( Values: 0 up to )
    *
    * Possible speed improvement: using bitshift instead of decimal operations. But interface stays the same, therefore can be done any time in the future
    */
    class FullSecID {
    public:
      bool operator<(const FullSecID& secID)  const { return m_fullSecID < secID.m_fullSecID; } /**< overloaded '<'-operator for sorting algorithms */
      bool operator==(const FullSecID& secID) const { return m_fullSecID == secID.m_fullSecID; } /**< overloaded '=='-operator for sorting algorithms */

      /** Copy constructor. */
      FullSecID(const FullSecID& secID):
        m_fullSecID(secID.m_fullSecID) {}

      /** Constructor taking the encoded full Sec ID (int).
      * use this one if you have got an already existing integer storing the FullSecID information and you want the possibility to extract its entries (layer, sublayer, vxdID, secID)
      *
      *  @param id  Number of the SecID using the encoded format. Careful - SecIDs are not continuously within the encoded number.
      *                The default is set to zero which represents the secID of the IP
      */
      FullSecID(unsigned int id = 0):
        m_fullSecID(id) {}


      /** Constructor taking the encoded full Sec ID (string).
      * use this one if you want to convert a string-encoded sector into the internal version (e.g. provided by the xml-files storing a sectormap) into the internal version. This constructor should be dispensable after changing from xml-based data storage to sql-storage
      *
      *  @param sid  global secID encoded in a string file eg "41_12345_3" 41 means layer 4, sublayer 1, 12345 is a dummy for the integer-version of the VxdID and the last digit is the local secID on the sensor
      */
      FullSecID(std::string sid);


      /** Constructor. If you want to create a new Full Sec ID using the information which shall be stored there
      * (e.g. layer, sublayer, vxdID, secID)
      *
      * @param vxdID  full VxdID of sensor of current sector.
      * @param subLayerID   0 if not in overlapping area of sensor, 1 if within overlap.
      * @param sectorNumber    number of sector at current sensor/vxdID.
      */
      FullSecID(VxdID vxdID, bool subLayerID, unsigned int sectorNumber);

      /** Another contstructor which does not need a VxdID nor another coded valid FullSecID (for testing purposes) */
      FullSecID(unsigned int layerID, bool subLayerID, unsigned int sensorID, unsigned int sectorNumber);

      /** Destructor. */
      ~FullSecID() {}

      /** returns LayerID compatible with basf2 standards. */
      short int getLayerID() { return getLayerNumber(); }

      /** returns LayerID compatible with basf2 standards. */
      short int getLayerNumber() { return m_fullSecID >> LayerBitShift; }

      /** returns LadderID compatible with basf2 standards */
      int getLadderID() { return getLadderNumber(); }

      /** returns LadderID compatible with basf2 standards */
      int getLadderNumber() { return getVxdID().getLadderNumber(); }

      /** returns SubLayerID which tells you whether it is useful to search for compatible sectors in the same layer as well. */
      bool getSubLayerID() { return (m_fullSecID bitand SubLayerMask) >> SubLayerBitShift; }

      /** returns VxdID of sensor. */
      VxdID getVxdID() { return VxdID((m_fullSecID bitand VxdIDMask) >> VxdIDBitShift); }

      /** returns uniID of sensor (basically the same as VxdID (can simply converted to vxdID(uniID)) but a trifle faster when only the short int value storing the VxdID information is wanted). */
      unsigned short int getUniID() { return (m_fullSecID bitand VxdIDMask) >> VxdIDBitShift; }

      /** returns SecID of current FullSecID (only unique for each sensor). */
      short int getSecID() { return (m_fullSecID bitand SectorMask); }

      /** returns the FullSecID coded as integer for further use (can be reconverted to FullSecID by using FullSecID(value)) */
      int getFullSecID() { return m_fullSecID; }

      /** returns the FullSecID coded as string compatible to secIDs stored in the xml-sectormaps */
      std::string getFullSecString();

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

      unsigned int m_fullSecID; /**< contains full info of current SecID. Check class definition for detailed description */

    }; //end class SecID
  } // end namespace Tracking
} //end namespace Belle2

#endif //FullSecID
