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

#ifndef PXDDAQSTATUS_H
#define PXDDAQSTATUS_H

// #include <framework/datastore/RelationsObject.h>
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
   *
   */
//   class PXDDAQStatus : public RelationsObject {
  class PXDDAQStatus : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQStatus() : m_errorMask(0), m_critErrorMask(0), m_usable(false) , m_gated(false), m_unfiltered(false) {}
    PXDDAQStatus(PXDErrorFlags m) : m_errorMask(m), m_critErrorMask(0), m_usable(false) , m_gated(false), m_unfiltered(false) {}

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

    /** Set flag that data was recorded without ROI filtering
     * @return
     */
    void setUnfiltered(bool b) { m_unfiltered = b; }

    /** Return if data was recorded without ROI filtering
     * @return data was not filtered flag
     */
    bool isUnfiltered() const { return m_unfiltered; }

    /** Set flag that data was recorded without ROI filtering
     * @return
     */
    void setGated(bool b) { m_gated = b; }

    /** Return if data was recorded without ROI filtering
     * @return data was not filtered flag
     */
    bool isGated() const { return m_gated; }

    /** Set Error bit mask
     * @return
     */
    void setErrorMask(PXDErrorFlags m) { m_errorMask = m; }

    /** Return Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getErrorMask(void) const { return m_errorMask; }

    /** Set Critical Error bit mask
     * @return
     */
    void setCritErrorMask(PXDErrorFlags m) { m_critErrorMask = m; }

    /** Return Critical Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getCritErrorMask(void) const { return m_critErrorMask; }

    /** Calculate the usability decision
     * @return
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == 0 && !m_gated;}

    void addPacket(PXDDAQPacketStatus& daqpktstat) {m_pxdPacket.push_back(daqpktstat);};

  private:
    PXDErrorFlags m_errorMask; /**< errors found in this DHC/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    bool m_usable; /**< data is useable.*/
    bool m_gated; /**< Sensor is in gated mode */
    bool m_unfiltered; /**< data was not ROI filtered */

    std::vector <PXDDAQPacketStatus> m_pxdPacket;

    ClassDef(PXDDAQStatus, 1)

  }; // class PXDDAQStatus


} // end namespace Belle2

#endif
