/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /* KLM forward declarations. */
  class KLMDigitRaw;

  namespace KLM {

    /**
     * Channel group.
     */
    struct ChannelGroup {

      /** First channel in the group. */
      int firstChannel = 0;

      /** Last channel in the group (0 for single-strip hits). */
      int lastChannel = 0;

      /** Strip number corresponding to the first channel. */
      int firstStrip = 0;

      /** Strip number corresponding to the last channel. */
      int lastStrip = 0;

    };

    /**
     * KLM raw data.
     */
    class RawData {

    public:

      /**
       * Constructor (unpack KLM raw data).
       * @param[in]     copper        Copper identifier.
       * @param[in]     slot          Slot number (1-based).
       * @param[in]     buffer        Data buffer (to be unpacked).
       * @param[in,out] klmDigitRaws  KLMDigitRaw array.
       * @param[out]    newDigitRaw   New KLMRawDigit.
       * @param[in]     fillDigitRaws Whether to fill klmDigitRaws.
       */
      RawData(
        int copper, int slot, const int* buffer,
        StoreArray<KLMDigitRaw>* klmDigitRaws, KLMDigitRaw** newDigitRaw,
        bool fillDigitRaws);

      /**
       * Destructor.
       */
      ~RawData();

      /**
       * Get lane.
       */
      uint16_t getLane() const
      {
        return m_Lane;
      }

      /**
       * Get axis.
       */
      uint16_t getAxis() const
      {
        return m_Axis;
      }

      /**
       * Get channel.
       */
      uint16_t getChannel() const
      {
        return m_Channel;
      }

      /**
       * Get CTIME.
       */
      uint16_t getCTime() const
      {
        return m_CTime;
      }

      /**
       * Get trigger bits.
       */
      uint16_t getTriggerBits() const
      {
        return m_TriggerBits;
      }

      /**
       * Get TDC.
       */
      uint16_t getTDC() const
      {
        return m_TDC;
      }

      /**
       * Get charge.
       */
      uint16_t getCharge() const
      {
        return m_Charge;
      }

      /**
       * Check whether this hit corresponds to multiple strips.
       */
      bool multipleStripHit() const
      {
        return (m_TriggerBits & 0x10) != 0;
      }

      /**
       * Get channel groups corresponding to this hit.
       * @param[out] channelGroups Channel groups.
       */
      void getChannelGroups(std::vector<ChannelGroup>& channelGroups) const;

      /**
       * Unpack lane.
       * @param[in] raw Raw-data word.
       */
      static uint16_t unpackLane(uint16_t raw)
      {
        return (raw >> 8) & 0x1F;
      }

      /**
       * Unpack axis.
       * @param[in] raw Raw-data word.
       */
      static uint16_t unpackAxis(uint16_t raw)
      {
        return (raw >> 7) & 0x1;
      };

      /**
       * Unpack channel.
       * @param[in] raw Raw-data word.
       */
      static uint16_t unpackChannel(uint16_t raw)
      {
        return raw & 0x7F;
      }

      /**
       * Unpack CTIME.
       * @param[in] raw Raw-data word.
       */
      static uint16_t unpackCTime(uint16_t raw)
      {
        return raw;
      }

      /**
       * Unpack trigger bits.
       * @param[in] raw Raw-data word.
       */
      static uint16_t unpackTriggerBits(uint16_t raw)
      {
        return (raw >> 11) & 0x1F;
      }

      /**
       * Unpack TDC.
       * @param[in] raw Raw-data word.
       */
      static uint16_t unpackTDC(uint16_t raw)
      {
        return raw & 0x7FF;
      }

      /**
       * Unpack charge.
       */
      static uint16_t unpackCharge(uint16_t raw)
      {
        return raw & 0xFFF;
      }

    protected:

      /** Lane (5 bits). */
      uint16_t m_Lane;

      /** Axis (1 bit). */
      uint16_t m_Axis;

      /** Channel (7 bits). */
      uint16_t m_Channel;

      /** CTIME (16 bits). */
      uint16_t m_CTime;

      /** Trigger bits (5 bits). */
      uint16_t m_TriggerBits;

      /** TDC (11 bits). */
      uint16_t m_TDC;

      /** Charge (12 bits). */
      uint16_t m_Charge;

    };

  }

}
