/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 * Created: 23.10.2017                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDDAQPacketStatus.h>
#include <TObject.h>
#include <pxd/dataobjects/PXDErrorFlags.h>

using namespace Belle2::PXD::PXDError;

namespace Belle2 {

  /**
   * The PXD DAQ Status class
   *
   * This is a small class that records information about PXD DAQ status
   * It will record if the data of PXD is useable.
   * In a tree structure, additional information per packet, DHC and DHE is
   * stored. This preserves information which would normally get lost
   * during unpacking
   */
  class PXDDAQStatus : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQStatus() : m_errorMask(0), m_critErrorMask(0), m_usable(false) , m_gated(false), m_unfiltered(false) {}

    /** constructor setting the error mask.
     * @param mask Error mask
     */
    PXDDAQStatus(PXDErrorFlags mask) : m_errorMask(mask), m_critErrorMask(0), m_usable(false) , m_gated(false), m_unfiltered(false) {}

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

    /** Set flag that data was recorded without ROI filtering
     * @param b unfiltered
     */
    void setUnfiltered(bool b) { m_unfiltered = b; }

    /** Return if data was recorded without ROI filtering
     * @return data was not ROI filtered flag
     */
    bool isUnfiltered() const { return m_unfiltered; }

    /** Set flag that data was while detector was gated
     * @param b gated
     */
    void setGated(bool b) { m_gated = b; }

    /** Return if data was recorded while detector was gated
     * @return detector was gated
     */
    bool isGated() const { return m_gated; }

    /** Set Error bit mask
     * This should be the OR of error masks of all sub-objects (DHC, DHE)
     * @param m Bit Mask to set
     */
    void setErrorMask(PXDErrorFlags m) { m_errorMask = m; }

    /** Return Error bit mask
     * This is the OR of error masks of all sub-objects (DHC, DHE)
     * @return bit mask
     */
    PXDErrorFlags getErrorMask(void) const { return m_errorMask; }

    /** Set Critical Error bit mask
     * @param m Bit Mask to set
     */
    void setCritErrorMask(PXDErrorFlags m) { m_critErrorMask = m; }

    /** Return Critical Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getCritErrorMask(void) const { return m_critErrorMask; }

    /** Calculate the usability decision
     * If any of the critical bits is set in the error mask, or the sensor was gated,
     * the PXD data is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == 0 && !m_gated;}

    /** Add Data packet information including its DHC/DHE tree
     * @param daqpktstat Packet Status Object
     */
    void addPacket(PXDDAQPacketStatus& daqpktstat) {m_pxdPacket.push_back(daqpktstat);};

    /** iterator-based iteration for packets */
    std::vector<PXDDAQPacketStatus>::const_iterator begin() const { return m_pxdPacket.begin(); };
    /** iterator-based iteration for packets */
    std::vector<PXDDAQPacketStatus>::const_iterator end() const { return m_pxdPacket.end(); };

  private:
    PXDErrorFlags m_errorMask; /**< errors found in any packet/DHC/DHE */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    bool m_usable; /**< Data is useable.*/
    bool m_gated; /**< Sensor was in gated mode */
    bool m_unfiltered; /**< Data was not ROI filtered */

    /** Vector of packet informations beloning to this event */
    std::vector <PXDDAQPacketStatus> m_pxdPacket;

    ClassDef(PXDDAQStatus, 1)

  }; // class PXDDAQStatus

} // end namespace Belle2
