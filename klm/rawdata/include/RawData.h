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

/* KLM headers. */
#include <klm/dataobjects/KLMDigitRaw.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

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

    };

    /**
     * Unpack KLM raw data.
     * @param[in]     buffer        Data buffer (to be unpacked).
     * @param[out]    data          Unpacked data.
     * @param[in,out] klmDigitRaws  KLMDigitRaw array.
     * @param[out]    newDigitRaw   New KLMRawDigit.
     * @param[in]     fillDigitRaws Whether to fill klmDigitRaws.
     */
    void unpackRawData(
      const int* buffer, RawData* data, StoreArray<KLMDigitRaw>* klmDigitRaws,
      KLMDigitRaw** newDigitRaw, bool fillDigitRaws);

  }

}
