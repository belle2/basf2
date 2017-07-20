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

  enum class SVDDAQModeType : uint8_t {
    zerosupp_6samples = 6,
    zerosupp_3samples = 3
  };

  std::ostream& operator<<(std::ostream& os, SVDDAQModeType mode);

  /**
   * Class to store mode information in an SVDShaperDigit.
   *
   * - Mode : Zero suppressed 6- or 3- samples + unknown mode
   * - Trigger time bin - 3 bits, correct values 0-3, otehr - error
   */
  class SVDModeByte {
  public:
    /** The base integer type for SVDModeByte */
    typedef uint8_t baseType;
    enum {
      /** Number of bits available to represent a triggerBin */
      TriggerBinBits = 3,
      /** Number of bits available to represent a daqMode */
      DAQModeBits = 3,
      /** Total bit size of the SVDModeByte */
      Bits = TriggerBinBits + DAQModeBits,

      /** Maximum available trigger time ID */
      MaxTruggerTime = (1 << TriggerBinBits) - 1,
      /** Maximum valid trigger time ID */
      MaxGoodTriggerBin = (1 << 2) - 1,
      /** Maximum valid DAQ mode ID */
      MaxDAQMode = (1 << DAQModeBits) - 1,
      /** Maximum value for ID */
      MaxID = (1 << Bits) - 1,

      /** Default / non-ifnformative id
       * Trigger 111 = 7, mode 6 samples
       */
      DefaultID  = 62
    };

    /** Constructor using the unique id.
     * Default set to represent a nonsensical value.
     */
    // cppcheck-suppress noExplicitConstructor
    SVDModeByte(baseType id = DefaultID)
    {
      m_id.id = id;
    }
    /** Constructor using triggerBin, daqMode and sensor ids */
    SVDModeByte(baseType triggerBin, SVDDAQModeType daqMode)
    {
      m_id.parts.triggerBin   = triggerBin;
      m_id.parts.daqMode  = daqMode;
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

    /** Set the unique id */
    void setID(baseType id) { m_id.id = id; }
    /** Set the triggerBin id */
    void setTriggerBin(baseType triggerBin) { m_id.parts.triggerBin  = triggerBin;  }
    /** Set the daqMode id */
    void setDAQMode(SVDDAQModeType daqMode) { m_id.parts.daqMode = daqMode; }

    /** make this type printable in python with print(vxd_id) */
    std::string __str__() const { return (std::string)(*this); }

  private:

    union {
      /** Unique id */
baseType id: Bits;
      struct {
        /** DAQ mode id */
SVDDAQModeType daqMode: DAQModeBits;
        /** Trigger time id */
baseType triggerBin: TriggerBinBits;
      } parts /**< Struct to contain all id components */;
    } m_id; /**< Union to store the ID and all components in one go. */
  };

  /** Print id to stream by converting it to string */
  std::ostream& operator<<(std::ostream& out, const SVDModeByte& id);
}

#endif //SVDMODEBYTE_H
