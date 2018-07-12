/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDID_H
#define VXDID_H

#include <string>
#include <ostream>
#include <stdexcept>

namespace Belle2 {
  /**
   * Class to uniquely identify a any structure of the PXD and SVD.
   *
   * - Ladder, Layer and Sensor all start at 1
   * - Ladders are numbered with increasing phi
   * - Sensors are numbered from front to back
   * - Segment ID is used for PXD to encode frame number. Currently no use for SVD.
   *
   * Internal use of a union gets rid of all the bit shifting which would be
   * neccessary to represent the id as one baseType and get all the
   * components out of it. Disadvantage is that it is not guaranteed to be
   * portable, but neither is bit shifting
   */
  class VxdID {
  public:
    /** The base integer type for VxdID */
    typedef unsigned short baseType;
    enum {
      /** Number of bits available to represent a layer */
      LayerBits   = 3,
      /** Number of bits available to represent a ladder */
      LadderBits  = 5,
      /** Number of bits available to represent a sensor */
      SensorBits  = 3,
      /** Number of bits available to represent segmentation of the sensor */
      SegmentBits = 5,
      /** Total bit size of the VxdID */
      Bits        = LayerBits + LadderBits + SensorBits + SegmentBits,

      /** Maximum valid Layer ID */
      MaxLayer    = (1 << LayerBits) - 1,
      /** Maximum valid Ladder ID */
      MaxLadder   = (1 << LadderBits) - 1,
      /** Maximum valid Sensor ID */
      MaxSensor   = (1 << SensorBits) - 1,
      /** Maximum valid Segment ID */
      MaxSegment  = (1 << SegmentBits) - 1,
      /** Maximum value for ID */
      MaxID       = (1 << Bits) - 1
    };

    /** Constructor using the unique id */
    // cppcheck-suppress noExplicitConstructor
    VxdID(baseType id = 0)
    {
      m_id.id = id;
    }
    /** Constructor using layer, ladder and sensor ids */
    VxdID(baseType layer, baseType ladder, baseType sensor, baseType segment = 0)
    {
      m_id.parts.layer   = layer;
      m_id.parts.ladder  = ladder;
      m_id.parts.sensor  = sensor;
      m_id.parts.segment = segment;
    }
    /** Construct ID from string representing the structure */
    explicit VxdID(const std::string& sensor);
    /** Copy constructor */
    VxdID(const VxdID& b): m_id(b.m_id) {}

    /** Assignment operator */
    VxdID& operator=(const VxdID& b)        { m_id = b.m_id; return *this; }
    /** Assignment from baseType */
    VxdID& operator=(baseType id)     { m_id.id = id; return *this; }
    /** Convert to baseType */
    operator baseType() const         { return getID(); }
    /** Convert to string */
    operator std::string() const;
    /** Check for equality */
    bool operator==(const VxdID& b) const   { return getID() == b.getID(); }
    /** Order by unique id */
    bool operator<(const VxdID& b) const    { return getID() < b.getID(); }

    /** Get the unique id */
    baseType getID() const            { return m_id.id; }
    /** Get the layer id */
    baseType getLayerNumber() const         { return m_id.parts.layer; }
    /** Get the ladder id */
    baseType getLadderNumber() const        { return m_id.parts.ladder; }
    /** Get the sensor id */
    baseType getSensorNumber() const        { return m_id.parts.sensor; }
    /** Get the sensor segment */
    baseType getSegmentNumber() const       { return m_id.parts.segment; }

    /** Set the unique id */
    void setID(baseType id)           { m_id.id = id; }
    /** Set the layer id */
    void setLayerNumber(baseType layer)     { m_id.parts.layer  = layer;  }
    /** Set the ladder id */
    void setLadderNumber(baseType ladder)   { m_id.parts.ladder = ladder; }
    /** Set the sensor id */
    void setSensorNumber(baseType sensor)   { m_id.parts.sensor = sensor; }
    /** Set the sensor segment */
    void setSegmentNumber(baseType segment) { m_id.parts.segment = segment; }

    /** make this type printable in python with print(vxd_id) */
    std::string __str__() const { return (std::string)(*this); }

  private:

    union {
      /** Unique id */
baseType id: Bits;
      struct {
        /** Segment id */
baseType segment: SegmentBits;
        /** Sensor id */
baseType sensor: SensorBits;
        /** Ladder id */
baseType ladder: LadderBits;
        /** Layer id */
baseType layer: LayerBits;
      } parts /**< Struct to contain all id components */;
    } m_id; /**< Union to store the ID and all components in one go. */
  };

  /** Print id to stream by converting it to string */
  std::ostream& operator<<(std::ostream& out, const VxdID& id);
}

#endif //VXDID_H
