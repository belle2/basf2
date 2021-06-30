/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  /** This class contains information to correct the information obtained from the TTD (simple offsets)
   */
  class TTDOffsets: public TObject {
  public:

    /** default constructor
     *  all offsets set to 0, use corresponding setters
     */
    TTDOffsets():
      m_injectionDelayHER(0), m_injectionDelayLER(0),
      m_triggeredBunchOffsetHER(0), m_triggeredBunchOffsetLER(0)
    { }

    /** Destructor */
    ~TTDOffsets() {}

    /** Set the delay between the pre-kick signal and arrival of bunch at IP for HER in clock ticks. */
    void setInjectionDelayHER(int delay) { m_injectionDelayHER = delay; }
    /** Set the delay between the pre-kick signal and arrival of bunch at IP for LER in clock ticks. */
    void setInjectionDelayLER(int delay) { m_injectionDelayLER = delay; }
    /** Set the offset between triggered bunch nr and SKB bunch nr for HER. */
    void setTriggeredBunchOffsetHER(int offset) { m_triggeredBunchOffsetHER = offset; }
    /** Set the offset between triggered bunch nr and SKB bunch nr for LER. */
    void setTriggeredBunchOffsetLER(int offset) { m_triggeredBunchOffsetLER = offset; }

    /** Get the delay between the pre-kick signal and arrival of bunch at IP for HER in clock ticks. */
    int getInjectionDelayHER() const { return m_injectionDelayHER; }
    /** Get the delay between the pre-kick signal and arrival of bunch at IP for LER in clock ticks. */
    int getInjectionDelayLER() const { return m_injectionDelayLER; }
    /** Get the offset between triggered bunch nr and SKB bunch nr for HER. */
    int getTriggeredBunchOffsetHER() const { return m_triggeredBunchOffsetHER; }
    /** Get the offset between triggered bunch nr and SKB bunch nr for LER. */
    int getTriggeredBunchOffsetLER() const { return m_triggeredBunchOffsetLER; }


  private:

    /** The delay between the time of the injection pre-kick signal and
     *  the time the noisy bunch passes by the detector for HER.
     *  Time in clock ticks (127MHz=RF/4) */
    int m_injectionDelayHER;

    /** The delay between the time of the injection pre-kick signal and
     *  the time the noisy bunch passes by the detector for LER.
     *  Time in clock ticks (127MHz=RF/4) */
    int m_injectionDelayLER;

    /** The number of the triggered bunch provided by the TriggerTimeDistribution (TTD)
     *  may be offset by some number wrt. the 'global' bunch numbering for HER. */
    int m_triggeredBunchOffsetHER;

    /** The number of the triggered bunch provided by the TriggerTimeDistribution (TTD)
     *  may be offset by some number wrt. the 'global' bunch numbering for LER. */
    int m_triggeredBunchOffsetLER;

    ClassDef(TTDOffsets, 1); /**< ROOTification **/
  };

} //Belle2 namespace
