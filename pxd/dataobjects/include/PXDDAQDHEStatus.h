/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDAQDHESTATUS_H
#define PXDDAQDHESTATUS_H

#include <vxd/dataobjects/VxdID.h>

// #include <framework/datastore/RelationsObject.h>
// #include <TObject.h>

#include <pxd/dataobjects/PXDErrorFlags.h>

using namespace Belle2::PXD::PXDError;


namespace Belle2 {

  /**
   * The PXD DAQ Status class
   *
   * This is a small class that records information about PXD DAQ DHE status
   * It will record if the data of this sensor (readout by this DHE) is useable.
   *
   */
//   class PXDDAQDHEStatus : public RelationsObject {
//   class PXDDAQDHEStatus : public TObject {
  class PXDDAQDHEStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHEStatus() : m_usable(true), m_errorMask(0), m_critErrorMask(0), m_rawCount(0), m_redCount(0) {}

    PXDDAQDHEStatus(VxdID id, int dheid, PXDErrorFlags mask, uint32_t raw, uint32_t red) : m_sensorID(id), m_dheID(dheid),
      m_usable(true), m_errorMask(mask), m_critErrorMask(0), m_rawCount(raw), m_redCount(red) {}

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

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
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == 0;}

    void setDHEID(VxdID id, int dheid) { m_sensorID = id; m_dheID = dheid;};

    void setCounters(uint32_t raw, uint32_t red) {m_rawCount = raw; m_redCount = red;};

  private:

    VxdID m_sensorID;/**< Sensor ID.*/
    unsigned short m_dheID;/**< DHE ID as delivered by DAQ.*/
    bool m_usable; /**< data is useable.*/
    PXDErrorFlags m_errorMask; /**< errors found in this DHE/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    uint32_t m_rawCount; /**< raw byte count for monitoring */
    uint32_t m_redCount; /**< reduced byte count for monitoring */

    ClassDef(PXDDAQDHEStatus, 1)

  }; // class PXDDAQDHEStatus


} // end namespace Belle2

#endif
