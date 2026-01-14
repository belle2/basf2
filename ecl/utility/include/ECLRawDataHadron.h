/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/**
 * This is a set of function for packing/unpacking the data produced by the
 * new ShaperDSP firmware that does hadron component fit.
 *
 * See 51st B2GM slides from ECL session, these docs and eclUnpackerModule.cc
 * for more details.
 *
 * Data format has changed to make space for the hadron fraction value.
 * Unlike the original ShaperDSP which sent fit amplitude and time as integer
 * values, new firmware sends amplitude and time as IEEE 754 floating point
 * values.
 */

namespace Belle2::ECL::RawDataHadron {

  /**
   * @brief Amplitude packing. See format description in unpackAmplitude docs.
   */
  unsigned long long packAmplitude(long long peak_amp);

  /**
   * @brief Amplitude unpacking (from 14 bits float to 18 bit int)
   *
   *  sign      -- not used, 0 bits
   *  exponent  -- 3 bits
   *  fraction  -- 11 bits
   */
  unsigned long long unpackAmplitude(unsigned long long amp_packed);

  /**
   * @brief Time packing. See format description in unpackTime docs.
   */
  int packTime(int peak_time);

  /**
   * @brief Time unpacking (from 11 bits float to 12 bits int)
   *
   *  sign     -- 1 bit
   *  exponent -- 2 bits
   *  fraction -- 8 bits
   */
  int unpackTime(int time_packed);

  /**
   * @brief Hadron fraction packing.
   *
   * Simply shifting (A_hadron / A_total) from [-0.25, 0.5) into [0, 31]
   */
  int packHadronFraction(int A_hadron, int A_total);

  /**
   * @brief Hadron fraction unpacking
   *
   * Simply shifting packed value from [0, 31] into [-0.25, 0.5)
   */
  double unpackHadronFraction(int fraction_packed);
}

#ifdef ECL_RAW_DATA_HADRON_STANDALONE_BUILD
using namespace Belle2::ECL::RawDataHadron;
#endif

