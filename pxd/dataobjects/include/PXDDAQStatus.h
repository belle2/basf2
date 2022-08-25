/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDDAQPacketStatus.h>
#include <TObject.h>
#include <pxd/dataobjects/PXDErrorFlags.h>

namespace Belle2 {


  using Belle2::PXD::PXDError::PXDErrorFlags;

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
    PXDDAQStatus() : m_errorMask(0), m_critErrorMask(0), m_usable(false), m_gated(false), m_unfiltered(false) {}

    /** constructor setting the error mask.
     * @param mask Error mask
     */
    explicit PXDDAQStatus(const PXDErrorFlags& mask) : m_errorMask(mask), m_critErrorMask(0), m_usable(false), m_gated(false),
      m_unfiltered(false) {}

    /** destructor */
    virtual ~PXDDAQStatus() {};

    /** Return pointer to PXDDAQDHEStatus for the DHE connected to sensor with VxdID id
     *  If id not found, it returns nullptr!
     * @param id VxdID for required DHE
     */
    const PXDDAQDHEStatus* findDHE(const VxdID& id) const;

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
     * @param mask Bit Mask to set
     */
    void setErrorMask(const PXDErrorFlags& mask) { m_errorMask = mask; }

    /** Add (OR) Error bit mask
     * This should be the OR of error masks of all sub-objects (DHC, DHE)
     * @param mask Bit Mask to add (or)
     */
    void addErrorMask(const PXDErrorFlags& mask) { m_errorMask |= mask; }

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
     * If any of the critical bits is set in the error mask, or the sensor was gated,
     * the PXD data is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == PXDErrorFlags(0) && !m_gated;}

    /** Add Data packet information including its DHC/DHE tree
     * @param daqpktstat Packet Status Object
     */
    void addPacket(PXDDAQPacketStatus& daqpktstat) {m_pxdPacket.push_back(daqpktstat);};

    /** Add new Packet information
     * @param params constructor parameter
     * @return ref to new Packet Status Object
     */
    template<class ...Args> PXDDAQPacketStatus& newPacket(Args&& ... params)
    {
      /*return*/ m_pxdPacket.emplace_back(std::forward<Args>(params)...);
      return m_pxdPacket.back();
    }

    /** Return map of module usability
     * @return map of VxdID with usable flags
     */
    std::map <VxdID, bool> getUsable() const;


    /** iterator-based iteration for packets */
    std::vector<PXDDAQPacketStatus>::iterator begin() { return m_pxdPacket.begin(); };
    /** iterator-based iteration for packets */
    std::vector<PXDDAQPacketStatus>::iterator end() { return m_pxdPacket.end(); };

    /** const iterator-based iteration for packets */
    std::vector<PXDDAQPacketStatus>::const_iterator cbegin() const { return m_pxdPacket.cbegin(); };
    /** const iterator-based iteration for packets */
    std::vector<PXDDAQPacketStatus>::const_iterator cend() const { return m_pxdPacket.cend(); };
    /** Returns PXDDAQPacketStatus for last packet in this event  */
    PXDDAQPacketStatus& pkt_back() { return m_pxdPacket.back(); };
    /** Returns number of packets in this event  */
    size_t pkt_size() const { return m_pxdPacket.size(); };

  private:
    PXDErrorFlags m_errorMask; /**< errors found in any packet/DHC/DHE */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    bool m_usable; /**< Data is useable.*/
    bool m_gated; /**< Sensor was in gated mode */
    bool m_unfiltered; /**< Data was not ROI filtered */

    /** Vector of packet informations beloning to this event */
    std::vector <PXDDAQPacketStatus> m_pxdPacket;

    ClassDef(PXDDAQStatus, 3)

  }; // class PXDDAQStatus

} // end namespace Belle2
