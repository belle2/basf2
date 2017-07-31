/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDMODEBYTE_H
#define SVDMODEBYTE_H

#include <cstdint>
#include <string>
#include <ostream>
#include <utility>

namespace Belle2 {

  /** Run time, 2-bits:
   * 00 raw,
   * 01 transparent,
   * 10 zero-suppressed,
   * 11 zero-suppressed + hit time finding
   */
  enum class SVDRunType : uint8_t {
    raw = 0,
    transparent = 1,
    zero_suppressed = 2,
    zero_suppressed_timefit = 3
  };

  /** output SVDRunType data */
  std::ostream& operator<<(std::ostream& os, SVDRunType mode);

  /** Event Type, 3 bits, separated into 1+2 bits:
   * 1. bit:
   * 0 TTD event (global run form FTSW),
   * 1 standalone event (local run)
   */
  enum class SVDEventType : uint8_t {
    global_run = 0,
    local_run = 1
  };

  /** Output SVDEventType data */
  std::ostream& operator<<(std::ostream& os, SVDEventType evt);

  /** 2+3 bit:
   * 00 1-sample,
   * 01 3-sample,
   * 10 6-sample
   */
  enum class SVDDAQModeType : uint8_t {
    daq_1sample = 0,
    daq_3samples = 1,
    daq_6samples = 2
  };

  /** Output SVDDAQModeType data */
  std::ostream& operator<<(std::ostream& os, SVDDAQModeType mode);

  /**
   * Class to store mode information in an SVDShaperDigit.
   *
   * - Mode : Zero suppressed 6- or 3- samples + unknown mode
   * - Trigger time bin - 3 bits, correct values 0-3, other - error
   */
  class SVDModeByte {
  public:
    /** The base integer type for SVDModeByte */
    typedef uint8_t baseType;
    enum {
      /** Number of bits available to represent a triggerBin */
      TriggerBinBits = 3,
      /** Number of bits available to represent DAQ Mode */
      DAQModeBits = 2,
      /** Number of bits available to represent event type */
      EventTypeBits = 1,
      /** Number of bits available to represent run type */
      RunTypeBits = 2,
      /** Total bit size of the SVDModeByte */
      Bits = TriggerBinBits + DAQModeBits + EventTypeBits + RunTypeBits,

      /** Maximum available trigger time ID */
      MaxTriggerTime = (1 << TriggerBinBits) - 1,
      /** Maximum valid trigger time ID */
      MaxGoodTriggerBin = 3,
      /** Maximum available DAQ mode ID */
      MaxDAQMode = (1 << DAQModeBits) - 1,
      /** Maximum available event type ID */
      MaxEventType = (1 << EventTypeBits) - 1,
      /** Maximum available run type ID */
      MaxRunType = (1 << RunTypeBits) - 1,
      /** Maximum value for ID */
      MaxID = (1 << Bits) - 1,

    };

    /** Default / non-informative id 10010111 = 151
     * Run type: zero-suppressed, 2
     * Event type: global run, 0
     * DAQ mode: 6-samples, 2
     * Trigger 111 = 7
     */
    static const baseType c_DefaultID;

    /** Constructor using the unique id.
     */
    // cppcheck-suppress noExplicitConstructor
    SVDModeByte(baseType id = c_DefaultID)
    {
      m_id.id = id;
    }
    /** Constructor using triggerBin, daqMode and sensor ids */
    SVDModeByte(SVDRunType runType, SVDEventType eventType, SVDDAQModeType daqMode, baseType triggerBin)
    {
      m_id.parts.triggerBin   = triggerBin;
      m_id.parts.daqMode  = daqMode;
      m_id.parts.eventType = eventType;
      m_id.parts.runType = runType;
    }
    /** Constructor using cast of baseType values */
    SVDModeByte(baseType runType, baseType eventType, baseType daqMode, baseType triggerBin)
    {
      m_id.parts.triggerBin   = triggerBin;
      m_id.parts.daqMode  = static_cast<SVDDAQModeType>(daqMode);
      m_id.parts.eventType = static_cast<SVDEventType>(eventType);
      m_id.parts.runType = static_cast<SVDRunType>(runType);
    }
    /** Copy constructor */
    SVDModeByte(const SVDModeByte& b): m_id(b.m_id) {}

    /** Assignment operator */
    SVDModeByte& operator=(const SVDModeByte& b) { m_id = b.m_id; return *this; }
    /** Assignment from baseType */
    SVDModeByte& operator=(baseType id) { m_id.id = id; return *this; }
    /** Convert to baseType */
    operator baseType() const { return getID(); }
    /** Convert to string */
    operator std::string() const;
    /** Check for equality */
    bool operator==(const SVDModeByte& b) const   { return getID() == b.getID(); }

    /** Get the unique id */
    baseType getID() const { return m_id.id; }
    /** Get the triggerBin id */
    baseType getTriggerBin() const { return m_id.parts.triggerBin; }
    /** Get the triggerBin id */
    std::pair<double, double> getTriggerInterval() const
    {
      baseType bin = m_id.parts.triggerBin;
      if (bin <= MaxGoodTriggerBin) {
        return std::make_pair(
                 - (1.0 - bin * 0.25) * 31.44,
                 - (0.75 - bin * 0.25) * 31.44
               );
      } else  // with no sensible info, return the whole interval
        return std::make_pair(-31.44, 0.0);
    }
    /** Get the daqMode id */
    SVDDAQModeType getDAQMode() const { return m_id.parts.daqMode; }
    /** Get the eventMode id */
    SVDEventType getEventType() const { return m_id.parts.eventType; }
    /** Get the runMode id */
    SVDRunType getRunType() const { return m_id.parts.runType; }

    // Some useful getters
    /** Do we have time fit information? */
    bool hasTimeFit() const
    { return m_id.parts.runType == SVDRunType::zero_suppressed_timefit; }
    /** Do we have useful run type?
     * A useful run is
     * - zero-suppressed (first bit set)
     * - 3-sample or 6-sample mode (daq mode > 0)
     */
    bool isUsefulRun() const
    {
      return (
               (
                 (m_id.parts.runType == SVDRunType::zero_suppressed)
                 ||
                 (m_id.parts.runType == SVDRunType::zero_suppressed_timefit)
               )
               &&
               (m_id.parts.daqMode != SVDDAQModeType::daq_1sample)
             );
    }

    /** Set the unique id */
    void setID(baseType id) { m_id.id = id; }
    /** Set the triggerBin id */
    void setTriggerBin(baseType triggerBin)
    { m_id.parts.triggerBin  = triggerBin;  }
    /** Set the daqMode id */
    void setDAQMode(SVDDAQModeType daqMode) { m_id.parts.daqMode = daqMode; }
    /** Set the eventType id */
    void setEventType(SVDEventType eventType) { m_id.parts.eventType = eventType; }
    /** Set the runType id */
    void setRunType(SVDRunType runType) { m_id.parts.runType = runType; }

    /** make this type printable in python with print(vxd_id) */
    std::string __str__() const { return (std::string)(*this); }

  private:

    // NB: the awful indentation is due to fixstyle.
    union {
      /** Unique id */
baseType id: Bits;
      struct {
        /** Trigger time id */
baseType triggerBin: TriggerBinBits;
        /** DAQ mode id */
SVDDAQModeType daqMode: DAQModeBits;
        /** Event type id */
SVDEventType eventType: EventTypeBits;
        /** Run type id */
SVDRunType runType: RunTypeBits;
      } parts /**< Struct to contain all id components */;
    } m_id; /**< Union to store the ID and all components in one go. */
  };

  /** Print id to stream by converting it to string */
  std::ostream& operator<<(std::ostream& out, const SVDModeByte& id);
} // namespace Belle2

#endif //SVDMODEBYTE_H
