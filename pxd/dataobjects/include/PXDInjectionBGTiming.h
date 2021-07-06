/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>


namespace Belle2 {

  /**
   * The PXDInjectionBGTiming class
   *
   * This is a small class that records information about injection vetos
   * for PXD related timing to emulate gated mode operation of the PXD.
   */

  class PXDInjectionBGTiming : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDInjectionBGTiming() : m_triggerGate(0), m_gated(false) {}

    /** Set flag that data was while detector was gated
     */
    void setGated(bool b) { m_gated = b; }

    /** Return if data was recorded while detector was gated
     */
    bool isGated() const { return m_gated; }

    /** Set trigger gate
     */
    void setTriggerGate(int g) { m_triggerGate = g; }

    /** Return trigger gate
     */
    int getTriggerGate(void) const { return m_triggerGate; }

    /** Return vector of start times for gating
     */
    const std::vector<float>& getGatingStartTimes(void) const { return m_gatingStartTimes; }

    /** Return vector of start times for gating
     */
    std::vector<float>& getGatingStartTimes(void) { return m_gatingStartTimes; }

  private:

    /** PXD triggergate */
    int m_triggerGate;

    /** Gated mode flag */
    bool m_gated;

    /** Start times for gating */
    std::vector<float> m_gatingStartTimes;

    ClassDef(PXDInjectionBGTiming, 1)

  }; // class PXDInjectionBGTiming

} // end namespace Belle2
