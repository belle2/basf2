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
   * - Segment ID is not used at the moment but foreseen for tracking and or
   *   alignment subdivisions of one sensor.
   *
   * Internal use of a union gets rid of all the bit shifting which would be
   * neccessary to represent the id as one unsigned short and get all the
   * components out of it. Disadvantage is that it is not guaranteed to be
   * portable, but neither is bit shifting
   */
  class VxdID {
  public:
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
    VxdID(unsigned short id = 0) {
      m_id.id = id;
    }
    /** Constructor using layer, ladder and sensor ids */
    VxdID(unsigned short layer, unsigned short ladder, unsigned short sensor, unsigned short segment = 0) {
      m_id.parts.layer   = layer;
      m_id.parts.ladder  = ladder;
      m_id.parts.sensor  = sensor;
      m_id.parts.segment = segment;
    }
    /** Construct ID from string representing the structure */
    VxdID(const std::string& sensor) throw(std::invalid_argument);
    /** Copy constructor */
    VxdID(const VxdID& b): m_id(b.m_id) {}

    /** Assignment operator */
    VxdID& operator=(const VxdID& b)        { m_id = b.m_id; return *this; }
    /** Assignment from unsigned short */
    VxdID& operator=(unsigned short id)     { m_id.id = id; return *this; }
    /** Convert to unsigned short */
    operator unsigned short() const         { return getID(); }
    /** Convert to string */
    operator std::string() const;
    /** Check for equality */
    bool operator==(const VxdID& b) const   { return getID() == b.getID(); }
    /** Order by unique id */
    bool operator<(const VxdID& b) const    { return getID() < b.getID(); }

    /** Get the unique id */
    unsigned short getID() const            { return m_id.id; }
    /** Get the layer id */
    unsigned short getLayerNumber() const         { return m_id.parts.layer; }
    /** Get the ladder id */
    unsigned short getLadderNumber() const        { return m_id.parts.ladder; }
    /** Get the sensor id */
    unsigned short getSensorNumber() const        { return m_id.parts.sensor; }
    /** Get the sensor segment */
    unsigned short getSegmentNumber() const       { return m_id.parts.segment; }

    /** Set the unique id */
    void setID(unsigned short id)           { m_id.id = id; }
    /** Set the layer id */
    void setLayerNumber(unsigned short layer)     { m_id.parts.layer  = layer;  }
    /** Set the ladder id */
    void setLadderNumber(unsigned short ladder)   { m_id.parts.ladder = ladder; }
    /** Set the sensor id */
    void setSensorNumber(unsigned short sensor)   { m_id.parts.sensor = sensor; }
    /** Set the sensor segment */
    void setSegmentNumber(unsigned short segment) { m_id.parts.segment = segment; }

  private:

    union {
      /** Unique id */
unsigned id: Bits;
      struct {
        /** Segment id */
unsigned segment: SegmentBits;
        /** Sensor id */
unsigned sensor: SensorBits;
        /** Ladder id */
unsigned ladder: LadderBits;
        /** Layer id */
unsigned layer: LayerBits;
      } parts /**< Struct to contain all id components */;
    } m_id; /**< Union to store the ID and all components in one go. */
  };

  /** Print id to stream by converting it to string */
  std::ostream& operator<<(std::ostream& out, const VxdID& id);
}

#endif //VXDID_H
