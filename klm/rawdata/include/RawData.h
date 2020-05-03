/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /* KLM forward declarations. */
  class KLMDigitRaw;

  namespace KLM {

    /**
     * KLM raw data.
     */
    struct RawData {

      /** Lane (5 bits, but only 3 are really necessary). */
      uint16_t lane;

      /** Axis (1 bit). */
      uint16_t axis;

      /** Channel (7 bits). */
      uint16_t channel;

      /** CTIME (16 bits). */
      uint16_t ctime;

      /** Trigger bits (5 bits). */
      uint16_t triggerBits;

      /** TDC (11 bits). */
      uint16_t tdc;

      /** Charge (12 bits). */
      uint16_t charge;

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
      static uint16_t unpackCtime(uint16_t raw)
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
      static uint16_t unpackTdc(uint16_t raw)
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

      /**
       * Check whether this hit corresponds to multiple strips.
       */
      bool multipleStripHit() const
      {
        return (triggerBits & 0x10) != 0;
      }

    };

    /**
     * Unpack KLM raw data.
     * @param[in]     copper        Copper identifier.
     * @param[in]     slot          Slot number (1-based).
     * @param[in]     buffer        Data buffer (to be unpacked).
     * @param[out]    data          Unpacked data.
     * @param[in,out] klmDigitRaws  KLMDigitRaw array.
     * @param[out]    newDigitRaw   New KLMRawDigit.
     * @param[in]     fillDigitRaws Whether to fill klmDigitRaws.
     */
    void unpackRawData(
      int copper, int slot, const int* buffer, RawData* data,
      StoreArray<KLMDigitRaw>* klmDigitRaws, KLMDigitRaw** newDigitRaw,
      bool fillDigitRaws);

  }

}
