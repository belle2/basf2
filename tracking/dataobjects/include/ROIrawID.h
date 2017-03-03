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


    explicit ROIrawID(baseType id = 0)
    {
      m_rawID.id = id;
    }; /**< constructor setting the id */

    ROIrawID(baseType SystemFlag, baseType DHHID, baseType RowMin, baseType ColMin, baseType RowMax, baseType ColMax)
    {

      m_rawID.parts.unused = 0;
      m_rawID.parts.systemFlag = SystemFlag;
      m_rawID.parts.DHHID = DHHID;
      m_rawID.parts.rowMin = RowMin;
      m_rawID.parts.colMin = ColMin;
      m_rawID.parts.rowMax = RowMax;
      m_rawID.parts.colMax = ColMax;
    } /**< constructor */

    baseType getSystemFlag() const {return m_rawID.parts.systemFlag;} /**< get system flag */
    baseType getDHHID() const {return m_rawID.parts.DHHID;} /**< get DHH ID*/
    baseType getRowMin() const {return m_rawID.parts.rowMin;} /**< get row min*/
    baseType getColMin() const {return m_rawID.parts.colMin;} /**< get col min*/
    baseType getRowMax() const {return m_rawID.parts.rowMax;} /**< get row max*/
    baseType getColMax() const {return m_rawID.parts.colMax;} /**< get col max*/
    baseType getID() const {return m_rawID.id;} /**< get ROIrawID*/

    baseType getBigEndian() const; /**< get bigEndian*/


    void setSystemFlag(baseType  SystemFlag) {m_rawID.parts.systemFlag = SystemFlag;} /**< set system flag*/
    void setDHHID(baseType DHHID) {m_rawID.parts.DHHID = DHHID;} /**< set DHH ID*/
    void setRowMin(baseType RowMin) {m_rawID.parts.rowMin = RowMin;} /**< set row min*/
    void setColMin(baseType ColMin) {m_rawID.parts.colMin = ColMin;} /**< set col min*/
    void setRowMax(baseType RowMax) {m_rawID.parts.rowMax = RowMax;} /**< set row max*/
    void setColMax(baseType ColMax) {m_rawID.parts.colMax = ColMax;} /**< set col max*/
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
baseType colMax: ColMaxBits;
baseType rowMax: RowMaxBits;
baseType colMin: ColMinBits;
baseType rowMin: RowMinBits;
baseType DHHID: DHHIDBits;
baseType systemFlag: SystemFlagBits;
baseType unused: UnusedBits;
      } parts;

    } m_rawID; /**< raw ID*/


  };
}
