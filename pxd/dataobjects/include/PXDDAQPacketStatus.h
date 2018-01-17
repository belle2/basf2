/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 * Created: Dez 2017                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDDAQDHCStatus.h>
#include <pxd/dataobjects/PXDErrorFlags.h>

using namespace Belle2::PXD::PXDError;

namespace Belle2 {

  /**
   * The PXD DAQ Packet Status class
   *
   * This is a small class that records information about PXD DAQ packet status
   * It will record if the data of DHC/DHE/sensors (readout by this packet) is useable.
   *
   */
  class PXDDAQPacketStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQPacketStatus() : m_errorMask(0), m_critErrorMask(0), m_index(0), m_usable(true) {}

    /** constructor setting the error mask and packet index.
     * @param inx packet index
     * @param mask Error mask
     */
    PXDDAQPacketStatus(unsigned short inx, PXDErrorFlags mask) : m_errorMask(mask), m_critErrorMask(0), m_index(inx), m_usable(true) {}

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

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
     * If any of the critical bits is set in the error mask
     * the PXD data from this packet is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == 0;}

    /** Return Packet index
     * @return packet index
     */
    unsigned short getPktIndex(void) const { return m_index;};

    /** Add DHC information including its DHE tree
     * @param daqdhcstat DHC Status Object
     */
    void addDHC(PXDDAQDHCStatus& daqdhcstat) {m_pxdDHC.push_back(daqdhcstat);};

    /** iterator-based iteration for DHCs */
    std::vector<PXDDAQDHCStatus>::iterator begin()  { return m_pxdDHC.begin(); };
    /** iterator-based iteration for DHCs */
    std::vector<PXDDAQDHCStatus>::iterator end()  { return m_pxdDHC.end(); };

    PXDDAQDHCStatus& dhc_back()  { return m_pxdDHC.back(); };
    size_t dhc_size() const { return m_pxdDHC.size(); };

  private:
    PXDErrorFlags m_errorMask; /**< errors found in this packets sensors */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    unsigned short m_index;/**< Packet index as delivered by DAQ.*/
    bool m_usable; /**< data is useable.*/

    /** Vector of DHC informations beloning to this event */
    std::vector <PXDDAQDHCStatus> m_pxdDHC;

    ClassDef(PXDDAQPacketStatus, 1);

  }; // class PXDDAQPacketStatus

} // end namespace Belle2
