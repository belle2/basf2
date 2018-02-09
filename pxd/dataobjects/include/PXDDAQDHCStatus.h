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

#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDDAQDHEStatus.h>
#include <pxd/dataobjects/PXDErrorFlags.h>

namespace Belle2 {

  /**
   * The PXD DAQ DHC Status class
   *
   * This is a small class that records information about PXD DAQ DHC status
   * It will record if the data of sensors (readout by this DHC) is useable.
   *
   */
  using Belle2::PXD::PXDError::PXDErrorFlags;

  class PXDDAQDHCStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHCStatus() : m_errorMask(0), m_critErrorMask(0), m_usable(true), m_dhcID(0), m_rawCount(0), m_redCount(0) {}

    /** constructor setting the error mask, dhcid, raw and reduced data counters
     * @param dhcid DHC id
     * @param mask error mask
     */
    PXDDAQDHCStatus(int dhcid, PXDErrorFlags mask) : m_errorMask(mask),
      m_critErrorMask(0), m_usable(true), m_dhcID(dhcid), m_rawCount(0), m_redCount(0) {}

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
     * the PXD data from this DHC is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == 0;}

    /** Get DHC ID */
    unsigned short getDHCID(void) const { return m_dhcID;};
    /** Set DHC ID*/
    void setDHCID(int dhcid) {m_dhcID = dhcid;};

    /** Set Data counters for reduction calculation */
    void setCounters(uint32_t raw, uint32_t red) {m_rawCount = raw; m_redCount = red;};
    /** Get Data counters for reduction calculation */
    void getCounters(uint32_t& raw, uint32_t& red) const { raw = m_rawCount; red = m_redCount;};
    /** Set Raw Data counter for reduction calculation */
    uint32_t getRawCnt(void) const { return m_rawCount;};
    /** Set Reduced Data counter for reduction calculation */
    uint32_t getRedCnt(void) const { return m_redCount;};


    /** Add DHE information
     * @param daqdhe DHE Status Object
     */
    void addDHE(PXDDAQDHEStatus& daqdhe) { m_pxdDHE.push_back(daqdhe);};

    /** Add new DHE information
     * @param params constructor parameter
     * @return new DHE Status Object
     */
    template<class ...Args> PXDDAQDHEStatus&  newDHE(Args&& ... params)
    {
      /*return*/ m_pxdDHE.emplace_back(std::forward<Args>(params)...);
      return m_pxdDHE.back();
    }

    /** iterator-based iteration for DHEs */
    std::vector<PXDDAQDHEStatus>::iterator begin()  { return m_pxdDHE.begin(); };
    /** iterator-based iteration for DHEs */
    std::vector<PXDDAQDHEStatus>::iterator end()  { return m_pxdDHE.end(); };

    PXDDAQDHEStatus& dhe_back()  { return m_pxdDHE.back(); };
    size_t dhe_size() const { return m_pxdDHE.size(); };

  private:
    PXDErrorFlags m_errorMask; /**< errors found in this DHC/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */
    bool m_usable; /**< data is useable.*/

    unsigned short m_dhcID;/**< DHC ID as delivered by DAQ.*/
    uint32_t m_rawCount; /**< raw byte count for monitoring */
    uint32_t m_redCount; /**< reduced byte count for monitoring */

    /** Vector of DHE informations belonging to this event */
    std::vector <PXDDAQDHEStatus> m_pxdDHE;

    ClassDef(PXDDAQDHCStatus, 1);

  }; // class PXDDAQDHCStatus

} // end namespace Belle2
