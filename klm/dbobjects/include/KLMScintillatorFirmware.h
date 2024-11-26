/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * KLM scintillator firmware.
   */
  class KLMScintillatorFirmware : public TObject {

  public:

    /**
     * Enumerator for the scintillator firmware version.
     */
    enum FirmwareVersion : unsigned int {

      /** Flag for marking an invalid version. Useful for identifying wrong payloads. */
      c_Invalid = 0,

      /** Flag for Phase 2 version. Kept for legacy. */
      c_Phase2 = 1,

      /** Flag for Run 1 (from 2019a to 2022b) */
      c_Run1 = 2,

      /** Flag for Run 1 (from 2023c). */
      c_Run2 = 3,

    };

    /**
     * Constructor.
     */
    KLMScintillatorFirmware() = default;

    /**
     * Specialized constructor.
     */
    explicit KLMScintillatorFirmware(FirmwareVersion firmwareVersion) :
      m_FirmwareVersion{firmwareVersion}
    {
    };

    /**
     * Destructor.
     */
    ~KLMScintillatorFirmware() = default;

    /**
     * Set firmware version.
     */
    void setFirmwareVersion(FirmwareVersion firmwareVersion)
    {
      m_FirmwareVersion = firmwareVersion;
    };

    /**
     * Get firmware version.
     */
    FirmwareVersion getFirmwareVersion() const
    {
      return m_FirmwareVersion;
    };

  private:

    /** KLM scintillator firmware version. */
    FirmwareVersion m_FirmwareVersion {c_Invalid};

    /** Class version. */
    ClassDef(KLMScintillatorFirmware, 1);

  };

}
