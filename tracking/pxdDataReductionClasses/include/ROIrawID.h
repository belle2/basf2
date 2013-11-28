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
    typedef uint64_t baseType;
    enum {
      UnusedBits = 21,
      SystemFlagBits = 1,
      DHHIDBits = 6,
      RowMinBits = 10,
      ColMinBits = 8,
      RowMaxBits = 10,
      ColMaxBits = 8,
      Bits = UnusedBits + SystemFlagBits + DHHIDBits + RowMinBits + ColMinBits + RowMaxBits + ColMaxBits,

      MaxSystemFlag = (1 << SystemFlagBits) - 1,
      MaxDHHID = (1 << DHHIDBits) - 1,
      MaxRowMin = (1 << RowMinBits) - 1,
      MaxColMin = (1 << ColMinBits) - 1,
      MaxRowMax = (1 << RowMaxBits) - 1,
      MaxColMax = (1 << ColMaxBits) - 1,
    };


    ROIrawID(baseType id = 0) {
      m_rawID.id = id;
    };

    ROIrawID(baseType SystemFlag, baseType DHHID, baseType RowMin, baseType ColMin, baseType RowMax, baseType ColMax) {

      m_rawID.parts.unused = 0;
      m_rawID.parts.systemFlag = SystemFlag;
      m_rawID.parts.DHHID = DHHID;
      m_rawID.parts.rowMin = RowMin;
      m_rawID.parts.colMin = ColMin;
      m_rawID.parts.rowMax = RowMax;
      m_rawID.parts.colMax = ColMax;
    }

    baseType getSystemFlag() const {return m_rawID.parts.systemFlag;}
    baseType getDHHID() const {return m_rawID.parts.DHHID;}
    baseType getRowMin() const {return m_rawID.parts.rowMin;}
    baseType getColMin() const {return m_rawID.parts.colMin;}
    baseType getRowMax() const {return m_rawID.parts.rowMax;}
    baseType getColMax() const {return m_rawID.parts.colMax;}
    baseType getID() const {return m_rawID.id;}

    baseType getBigEndian() const;


    void setSystemFlag(baseType  SystemFlag) {m_rawID.parts.systemFlag = SystemFlag;}
    void setDHHID(baseType DHHID) {m_rawID.parts.DHHID = DHHID;}
    void setRowMin(baseType RowMin) {m_rawID.parts.rowMin = RowMin;}
    void setColMin(baseType ColMin) {m_rawID.parts.colMin = ColMin;}
    void setRowMax(baseType RowMax) {m_rawID.parts.rowMax = RowMax;}
    void setColMax(baseType ColMax) {m_rawID.parts.colMax = ColMax;}
    void setID(baseType id) {m_rawID.id = id;}


    ~ROIrawID() {};

    //operator used to order the ROIs (ascending DHHID)
    inline bool operator()(const ROIrawID& roi1, const ROIrawID& roi2) const {
      if (roi1.getDHHID() == roi2.getDHHID())
        return roi1.getID() < roi2.getID();
      else return roi1.getDHHID() < roi2.getDHHID();

    }

  private:

    union {
baseType id: Bits;
      struct {
baseType colMax: ColMaxBits;
baseType rowMax: RowMaxBits;
baseType colMin: ColMinBits;
baseType rowMin: RowMinBits;
baseType DHHID: DHHIDBits;
baseType systemFlag: SystemFlagBits;
baseType unused: UnusedBits;
      } parts;

    } m_rawID;

  };
}
