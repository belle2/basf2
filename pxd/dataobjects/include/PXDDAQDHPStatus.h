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
  class PXDDAQDHPStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHPStatus() : m_chipID(0), m_frameNr(0) {}

    /** constructor setting the error mask, dhcid, raw and reduced data counters, ...
     * @param chipid DHP chip id (2 bit)
     * @param fn (absolute) Readout Frame Number, lower 16 bits only
     */
    PXDDAQDHPStatus(uint8_t chipid, uint16_t fn) : m_chipID(chipid), m_frameNr(fn) {}

    /** Set Chip ID of DHP */
    void setChipID(uint8_t chipid) { m_chipID = chipid;};
    /** Get Chip ID of DHP */
    uint8_t getChipID(void) const { return  m_chipID;};

    /** set Readout Frame number */
    void setFrameNr(uint16_t fn) { m_frameNr = fn;};
    /** get Readout Frame number */
    uint16_t getFrameNr(void) const { return  m_frameNr;};

  private:

    uint8_t m_chipID;/**< Chip ID as delivered by DAQ.*/
    uint16_t m_frameNr; /**< Frame number (low bits) from DHP header */

    ClassDef(PXDDAQDHPStatus, 1);

  }; // class PXDDAQDHPStatus

} // end namespace Belle2
