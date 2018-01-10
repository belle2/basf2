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

#include <pxd/dataobjects/PXDDAQDHEStatus.h>

#include <pxd/dataobjects/PXDErrorFlags.h>

using namespace Belle2::PXD::PXDError;


namespace Belle2 {

  /**
   * The PXD DAQ Status class
   *
   * This is a small class that records information about PXD DAQ DHC status
   * It will record if the data of this sensor (readout by this DHC) is useable.
   *
   */
  class PXDDAQDHCStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHCStatus() : m_usable(true), m_errorMask(0), m_critErrorMask(0), m_rawCount(0), m_redCount(0) {}

    PXDDAQDHCStatus(int dhcid, PXDErrorFlags mask, uint32_t raw, uint32_t red) : m_dhcID(dhcid), m_usable(true), m_errorMask(mask),
      m_critErrorMask(0), m_rawCount(raw), m_redCount(red) {}

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

    void addDHEs(std::vector<PXDDAQDHEStatus>& daqdhevect)
    {
      for (auto a : daqdhevect) m_pxdDHE.push_back(a);
    };


    unsigned short getDHCID(void) const { return m_dhcID;};
    void setDHCID(int dhcid) {m_dhcID = dhcid;};

    void setCounters(uint32_t raw, uint32_t red) {m_rawCount = raw; m_redCount = red;};
    void getCounters(uint32_t& raw, uint32_t& red) const { raw = m_rawCount; red = m_redCount;};
    uint32_t getRawCnt(void) const { return m_rawCount;};
    uint32_t getRedCnt(void) const { return m_redCount;};

    //iterator-based iteration
    std::vector<PXDDAQDHEStatus>::const_iterator begin() const { return m_pxdDHE.begin(); };
    std::vector<PXDDAQDHEStatus>::const_iterator end() const { return m_pxdDHE.end(); };

  private:

    unsigned short m_dhcID;/**< DHC ID as delivered by DAQ.*/
    bool m_usable; /**< data is useable.*/

    PXDErrorFlags m_errorMask; /**< errors found in this DHC/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    uint32_t m_rawCount; /**< raw byte count for monitoring */
    uint32_t m_redCount; /**< reduced byte count for monitoring */

    std::vector <PXDDAQDHEStatus> m_pxdDHE;

    ClassDef(PXDDAQDHCStatus, 1);

  }; // class PXDDAQDHCStatus


} // end namespace Belle2
