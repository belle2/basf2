/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Björn Spruck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
