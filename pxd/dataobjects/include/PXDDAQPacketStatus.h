/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDDAQDHCStatus.h>
#include <pxd/dataobjects/PXDErrorFlags.h>

namespace Belle2 {


  using Belle2::PXD::PXDError::PXDErrorFlags;

  /**
   * The PXD DAQ Packet Status class
   *
   * This is a small class that records information about PXD DAQ packet status
   * It will record if the data of DHC/DHE/sensors (readout by this packet) is useable.
   *
   */
  class PXDDAQPacketStatus final {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQPacketStatus() : m_errorMask(0), m_critErrorMask(0), m_usable(true), m_index(0) {}

    /** constructor setting the error mask and packet index.
     * @param inx packet index
     */
    explicit PXDDAQPacketStatus(unsigned short inx) : m_errorMask(0), m_critErrorMask(0), m_usable(true), m_index(inx) {}

    /** destructor */
    virtual ~PXDDAQPacketStatus() {};

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

    /** Set Error bit mask
     * This should be the OR of error masks of all sub-objects (DHC, DHE)
     * @param mask Bit Mask to set
     */
    void setErrorMask(const PXDErrorFlags& mask) { m_errorMask = mask; }

    /** Return Error bit mask
     * This is the OR of error masks of all sub-objects (DHC, DHE)
     * @return bit mask
     */
    PXDErrorFlags getErrorMask(void) const { return m_errorMask; }

    /** Set Critical Error bit mask
     * @param mask Bit Mask to set
     */
    void setCritErrorMask(const PXDErrorFlags& mask) { m_critErrorMask = mask; }

    /** Return Critical Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getCritErrorMask(void) const { return m_critErrorMask; }

    /** Calculate the usability decision
     * If any of the critical bits is set in the error mask
     * the PXD data from this packet is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == PXDErrorFlags(0);}

    /** Set Packet index
     * @param inx packet index
     */
    void setPktIndex(unsigned short inx) { m_index = inx;};

    /** Return Packet index
     * @return packet index
     */
    unsigned short getPktIndex(void) const { return m_index;};

    /** Add DHC information including its DHE tree
     * @param daqdhcstat DHC Status Object
     */
    void addDHC(PXDDAQDHCStatus& daqdhcstat) {m_pxdDHC.push_back(daqdhcstat);};

    /** Add new DHC information
     * @param params constructor parameter
     * @return ref to new DHC Status Object
     */
    template<class ...Args> PXDDAQDHCStatus& newDHC(Args&& ... params)
    {
      /*return*/ m_pxdDHC.emplace_back(std::forward<Args>(params)...);
      return m_pxdDHC.back();
    }

    /** iterator-based iteration for DHCs */
    std::vector<PXDDAQDHCStatus>::iterator begin()  { return m_pxdDHC.begin(); };
    /** iterator-based iteration for DHCs */
    std::vector<PXDDAQDHCStatus>::iterator end()  { return m_pxdDHC.end(); };

    /** const iterator-based iteration for DHCs */
    std::vector<PXDDAQDHCStatus>::const_iterator cbegin() const { return m_pxdDHC.cbegin(); };
    /** const iterator-based iteration for DHCs */
    std::vector<PXDDAQDHCStatus>::const_iterator cend() const { return m_pxdDHC.cend(); };
    /** Returns PXDDAQDHCStatus for last DHC */
    PXDDAQDHCStatus& dhc_back()  { return m_pxdDHC.back(); };
    /** Returns number of DHCs */
    size_t dhc_size() const { return m_pxdDHC.size(); };

  private:
    PXDErrorFlags m_errorMask; /**< errors found in this packets sensors */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */
    bool m_usable; /**< data is useable.*/

    unsigned short m_index;/**< Packet index as delivered by DAQ.*/

    /** Vector of DHC informations beloning to this event */
    std::vector <PXDDAQDHCStatus> m_pxdDHC;

    /** necessary for ROOT */
    ClassDef(PXDDAQPacketStatus, 3);

  }; // class PXDDAQPacketStatus

} // end namespace Belle2
