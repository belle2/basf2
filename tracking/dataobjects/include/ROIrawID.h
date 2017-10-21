/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <stdint.h>

namespace Belle2 {

  /** ROIrawID.
   *
   * 64 bit union that contains the ROI information to be stored in the
   * payload that is sent to the ONSEN.
   */
  class ROIrawID {

  public:
    //    typedef unsigned long baseType;
    typedef uint64_t baseType; /**< base type*/
    enum {
      UnusedBits = 21,
      SystemFlagBits = 1,
      DHHIDBits = 6,
      MinVBits = 10,
      MinUBits = 8,
      MaxVBits = 10,
      MaxUBits = 8,
      Bits = UnusedBits + SystemFlagBits + DHHIDBits + MinVBits + MinUBits + MaxVBits + MaxUBits,

// the following are unused atm, might be used for consistency checks
//       MaxSystemFlag = (1 << SystemFlagBits) - 1,
//       MaxDHHID = (1 << DHHIDBits) - 1,
//       MaxMinV = (1 << MinVBits) - 1,
//       MaxMinU = (1 << MinUBits) - 1,
//       MaxMaxV = (1 << MaxVBits) - 1,
//       MaxMaxU = (1 << MaxUBits) - 1,
    };


    explicit ROIrawID(baseType id = 0)
    {
      m_rawID.id = id;
    }; /**< constructor setting the id */

    ROIrawID(baseType SystemFlag, baseType DHHID, baseType MinV, baseType MinU, baseType MaxV, baseType MaxU)
    {

      m_rawID.parts.unused = 0;
      m_rawID.parts.systemFlag = SystemFlag;
      m_rawID.parts.DHHID = DHHID;
      m_rawID.parts.minV = MinV;
      m_rawID.parts.minU = MinU;
      m_rawID.parts.maxV = MaxV;
      m_rawID.parts.maxU = MaxU;
    } /**< constructor */

    baseType getSystemFlag() const {return m_rawID.parts.systemFlag;} /**< get system flag */
    baseType getDHHID() const {return m_rawID.parts.DHHID;} /**< get DHH ID*/
    baseType getMinVid() const {return m_rawID.parts.minV;} /**< get minimum V*/
    baseType getMinUid() const {return m_rawID.parts.minU;} /**< get minimum U*/
    baseType getMaxVid() const {return m_rawID.parts.maxV;} /**< get maximum V*/
    baseType getMaxUid() const {return m_rawID.parts.maxU;} /**< get maximum U*/
    baseType getID() const {return m_rawID.id;} /**< get ROIrawID*/

    baseType getBigEndian() const; /**< get bigEndian*/


    void setSystemFlag(baseType  SystemFlag) {m_rawID.parts.systemFlag = SystemFlag;} /**< set system flag*/
    void setDHHID(baseType DHHID) {m_rawID.parts.DHHID = DHHID;} /**< set DHH ID*/
    void setMinVid(baseType MinV) {m_rawID.parts.minV = MinV;} /**< set minimum V*/
    void setMinUid(baseType MinU) {m_rawID.parts.minU = MinU;} /**< set minimum U*/
    void setMaxVid(baseType MaxV) {m_rawID.parts.maxV = MaxV;} /**< set maximum V*/
    void setMaxUid(baseType MaxU) {m_rawID.parts.maxU = MaxU;} /**< set maximum U*/
    void setID(baseType id) {m_rawID.id = id;} /**< set ROIrawID*/


    ~ROIrawID() {};

    inline bool operator()(const ROIrawID& roi1, const ROIrawID& roi2) const
    {
      if (roi1.getDHHID() == roi2.getDHHID())
        return roi1.getID() < roi2.getID();
      else return roi1.getDHHID() < roi2.getDHHID();
    } /**< operator used to order the ROIs (ascending DHHID)*/

  private:

    union {
baseType id: Bits;
      struct {
baseType maxU: MaxUBits;
baseType maxV: MaxVBits;
baseType minU: MinUBits;
baseType minV: MinVBits;
baseType DHHID: DHHIDBits;
baseType systemFlag: SystemFlagBits;
baseType unused: UnusedBits;
      } parts;

    } m_rawID; /**< raw ID*/


  };
}
