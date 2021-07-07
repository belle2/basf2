/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /*! Class to store ECLTrig, still need to be study
   * relation to ECLHit
   * filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   * and ecl/modules/eclUnpacker/ECLUnpacker.cc
   */

  class ECLTrig : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLTrig()
    {
      m_TrigId = 0;    /**< Trig ID == crate ID */
      m_TimeTrig = 0; /**< Trig Time */
      m_TrigTag = 0;
    }


    /*! Set  TrigID
     */
    void setTrigId(int TrigId) { m_TrigId = TrigId; }
    /*! Set Trig Time (crate Id)
     */
    void setTrigTag(int TrigTag) { m_TrigTag = TrigTag; }
    /*! Set Triger Tag (crate Id)
     */
    void setTimeTrig(double TimeTrig) { m_TimeTrig = TimeTrig; }
    /*! Get Trig ID
     * @return trig ID
     */
    int getTrigId() const { return m_TrigId & 0x3F; }

    /*!
     * Burst suppression is an algorithm implemented in ECL ShaperDSP modules
     * to prevent buffer overflow in cases of beam burst.
     * In such case, ShaperDSP discards waveform data (ECLDsp) and sends
     * only fit results (ECLDigit).
     *
     * ALGORITHM DESCRIPTION
     *
     *   Each saved waveform increases beam burst suppression
     *   counter by 11 us.
     *   When counter passes the threshold of 50 us, waveforms
     *   are no longer saved until counter is below 50 us again.
     *
     *       ------------------>
     *
     *      \ +11 us /  \ +11 us /
     *       \~~~~~~/    \~~~~~~/
     *        \~~~~/      \~~~~/
     *         \__/        \__/
     *     =======================
     *                            +   Value of beam   +
     *                            + burst suppression +
     *                            +      counter      +
     *                            +                   +
     *                            +                   +
     *                            +                   +
     *                            +                   +  Block waveform data
     *                            +                   +  ^
     *                            +                   +  |
     *                      50 us +.......................
     *                            +                   +  |
     *                            +~~~~~~~~~~~~~~~~~~~+  v
     *                            +~~~~~~~~~~~~~~~~~~~+  Do not block
     *                            +~~~~~~~~~~~~~~~~~~~+  waveform data
     *                            +~~~~~~~~~~~~~~~~~~~+
     *                            +++++++++++++++++++++++>
     *                                                   🌢 -1 us
     *
     *                                                   🌢 -1 us
     *
     * @brief Return burst suppression mask. 0--suppression inactive, 1--active.
     * @return Burst suppression mask (12 bits, 1 bit per each ShaperDSP)
     */
    int getBurstSuppressionMask() const { return (m_TrigId >> 6) & 0xFFF; }

    /*! Get Trigger tag
     * Trigger tag word width is 16 bit (bits 0-15).
     * Bit 16 is used as error flag.
     * If it is 1 then trigger tags from different shapers are not equal
     * @return Trigger tag
     */
    int getTrigTag() const { return m_TrigTag & 0xFFFF; }

    /*! Get trigger tag quality flag.
     * @return 0 if trigger tags from different shapers are equal. 1 otherwise.
     */
    int getTrigTagQualityFlag() const { return (m_TrigTag & 0x10000) >> 16; }

    /*! Get Trig Time
     * @return Trig Time
     */
    double getTimeTrig() const { return m_TimeTrig; }

  private:
    int m_TrigId;      /**< Trig ID */
    double m_TimeTrig; /**< Trig Time */
    int m_TrigTag;     /**< Trig tag */


    /** 2 CellId -> TrigIdvariable lengthx */
    /** 3 TrgTag added */
    ClassDef(ECLTrig, 3);

  };
} // end namespace Belle2

