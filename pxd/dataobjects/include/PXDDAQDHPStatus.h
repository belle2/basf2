/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/// TClass is not used in this class, but the include is needed for the ClassDef
#include <TClass.h>

namespace Belle2 {

  /**
   * The PXD DAQ DHP Status class
   *
   * This is a small class that records information about PXD DAQ DHP status
   * It will record some stat data of this DHP
   *
   */
  class PXDDAQDHPStatus final {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHPStatus() : m_chipID(0), m_frameNr(0), m_truncated(false) {}

    /** constructor setting the error mask, dhcid, raw and reduced data counters, ...
     * @param chipid DHP chip id (2 bit)
     * @param fn (absolute) Readout Frame Number, lower 16 bits only
     */
    explicit PXDDAQDHPStatus(uint8_t chipid, uint16_t fn) : m_chipID(chipid), m_frameNr(fn) {}

    /** destructor */
    virtual ~PXDDAQDHPStatus() {};

    /** Set Chip ID of DHP */
    void setChipID(uint8_t chipid) { m_chipID = chipid;};
    /** Get Chip ID of DHP */
    uint8_t getChipID(void) const { return  m_chipID;};

    /** set Readout Frame number */
    void setFrameNr(uint16_t fn) { m_frameNr = fn;};
    /** get Readout Frame number */
    uint16_t getFrameNr(void) const { return  m_frameNr;};

    /** set Truncation */
    void setTruncated(void) { m_truncated = true;};
    /** get Truncation */
    bool getTruncated(void) { return  m_truncated;};

  private:

    uint8_t m_chipID{0};/**< Chip ID as delivered by DAQ.*/
    uint16_t m_frameNr{0}; /**< Frame number (low bits) from DHP header */
    bool m_truncated{false}; /**< DHE reports truncated frame */

    /** necessary for ROOT */
    ClassDef(PXDDAQDHPStatus, 2);

  }; // class PXDDAQDHPStatus

} // end namespace Belle2
