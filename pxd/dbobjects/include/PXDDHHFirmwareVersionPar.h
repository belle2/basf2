/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>


namespace Belle2 {

  /** The payload stores the firmware version from the DHH, which has influence on
   *  the structure and content of the RawPXD package. Needed for proper
   *  unpacking and error detection, crosschecks
   */

  class PXDDHHFirmwareVersionPar: public TObject {
  public:

    /** Default constructor */
    PXDDHHFirmwareVersionPar() : m_DHHFirmwareVersion(0) {}

    /** Set DHH Firmware Version
     */
    void setDHHFirmwareVersion(int DHHFirmwareVersion) { m_DHHFirmwareVersion = DHHFirmwareVersion; }

    /** Get DHH Firmware Version
     */
    int getDHHFirmwareVersion() const { return m_DHHFirmwareVersion; }


  private:

    /** DHH/DHE FirmwareVersion */
    int m_DHHFirmwareVersion;

    ClassDef(PXDDHHFirmwareVersionPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
