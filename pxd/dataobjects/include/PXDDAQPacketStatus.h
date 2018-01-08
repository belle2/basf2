/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>

// #include <framework/datastore/RelationsObject.h>
// #include <TObject.h>
#include <pxd/dataobjects/PXDDAQDHCStatus.h>

#include <pxd/dataobjects/PXDErrorFlags.h>

using namespace Belle2::PXD::PXDError;


namespace Belle2 {

  /**
   * The PXD DAQ Status class
   *
   * This is a small class that records information about PXD DAQ Packet status
   * It will record if the data of this sensor (readout by this Packet) is useable.
   *
   */
//   class PXDDAQPacketStatus : public RelationsObject {
//   class PXDDAQPacketStatus : public TObject {
  class PXDDAQPacketStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQPacketStatus() : m_errorMask(0), m_critErrorMask(0), m_index(0), m_usable(true) {}

    PXDDAQPacketStatus(unsigned short inx, PXDErrorFlags mask) : m_errorMask(mask), m_critErrorMask(0), m_index(inx), m_usable(true) {}

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

    void addDHC(PXDDAQDHCStatus& daqdhcstat) {m_pxdDHC.push_back(daqdhcstat);};

    //iterator-based iteration
    std::vector<PXDDAQDHCStatus>::const_iterator dhc_begin() const { return m_pxdDHC.begin(); };
    std::vector<PXDDAQDHCStatus>::const_iterator dhc_end() const { return m_pxdDHC.end(); };

    unsigned short getPktIndex(void) const { return m_index;};

  private:
    PXDErrorFlags m_errorMask; /**< errors found in this Packet/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    unsigned short m_index;/**< Packet index as delivered by DAQ.*/
    bool m_usable; /**< data is useable.*/

    std::vector <PXDDAQDHCStatus> m_pxdDHC;

    // ClassDef(PXDDAQPacketStatus, 1)

  }; // class PXDDAQPacketStatus


} // end namespace Belle2
