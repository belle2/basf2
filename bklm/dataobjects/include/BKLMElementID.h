/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMElementID_H
#define BKLMElementID_H

#include <string>
#include <ostream>
#include <stdexcept>

namespace Belle2 {
  /**
   * Class to uniquely identify a alignable element of BKLM.
   *
   * - isForward: 1 = forward; 0 = backward
   * - Sector, Layer all start at 1
   * - Sectors are numbered with increasing phi 1 - 8
   * - Layers are numbered from inner to out 1 - 15
   */
  class BKLMElementID {
  public:
    /** The base integer type for BKLMElementID */
    typedef unsigned short baseType;
    enum {
      /** Number of bits available to represent forward or backword*/
      IsForwardBits  = 1,
      /** Number of bits available to represent a sector */
      SectorBits   = 5,
      /** Number of bits available to represent a layer */
      LayerBits    = 8,
      /** Total bit size of the BKLMElementID */
      Bits        = IsForwardBits + SectorBits + LayerBits,

      /** Maximum valid forward/backward ID */
      MaxIsForward    = (1 << IsForwardBits) - 1,
      /** Maximum valid Sector ID */
      MaxSector   = (1 << SectorBits) - 1,
      /** Maximum valid Layer ID */
      MaxLayer    = (1 << LayerBits) - 1,
      /** Maximum value for ID */
      MaxID       = (1 << Bits) - 1
    };

    /** Constructor using the unique id */
    /** cppcheck suppress noExplicitConstructor */
    explicit BKLMElementID(baseType id = 0)
    {
      m_id.id = id;
    }
    /** Constructor using isForward, sector, layer ids */
    BKLMElementID(baseType isForward, baseType sector, baseType layer)
    {
      m_id.parts.isForward  = isForward;
      m_id.parts.sector  = sector;
      m_id.parts.layer   = layer;
    }

    /** Construct ID from string representing the structure */
    explicit BKLMElementID(const std::string& module);

    /** Copy constructor */
    BKLMElementID(const BKLMElementID& b): m_id(b.m_id) {}

    /** Assignment operator */
    BKLMElementID& operator=(const BKLMElementID& b)        { m_id = b.m_id; return *this; }

    /** Assignment from baseType */
    BKLMElementID& operator=(baseType id)     { m_id.id = id; return *this; }

    /** Convert to baseType */
    operator baseType() const         { return getID(); }

    /** Convert to string */
    operator std::string() const;

    /** Check for equality */
    bool operator==(const BKLMElementID& b) const   { return getID() == b.getID(); }

    /** Order by unique id */
    bool operator<(const BKLMElementID& b) const    { return getID() < b.getID(); }

    /** Get the unique id */
    baseType getID() const            { return m_id.id; }
    /** Get the IsForward */
    baseType getIsForward() const        { return m_id.parts.isForward; }
    /** Get the sector id */
    baseType getSectorNumber() const        { return m_id.parts.sector; }
    /** Get the layer id */
    baseType getLayerNumber() const         { return m_id.parts.layer; }

    /** Set the unique id */
    void setID(baseType id)           { m_id.id = id; }
    /** Set forward or backward */
    void setIsForward(baseType isForward)   { m_id.parts.isForward = isForward; }
    /** Set the sector id */
    void setSectorNumber(baseType sector)   { m_id.parts.sector = sector; }
    /** Set the layer id */
    void setLayerNumber(baseType layer)     { m_id.parts.layer  = layer;  }

  private:

    union {
      /** Unique id */
baseType id: Bits;
      struct {
        /** forward or backward */
baseType isForward: IsForwardBits;
        /** sector id */
baseType sector: SectorBits;
        /** Layer id */
baseType layer: LayerBits;
      } parts; /**< Struct to contain all id components */
    } m_id; /**< Union to store the ID and all components in one go. */
  };

  /** Print id to stream by converting it to string */
  std::ostream& operator<<(std::ostream& out, const BKLMElementID& id);
}

#endif //BKLMElementID_H
