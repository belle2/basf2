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

  /**
   * ARICH firmware.
   */
  class ARICHFirmware: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHFirmware(): m_comment(""), m_version(0), m_firmware("") {};

    /**
     * Constructor.
     */
    explicit ARICHFirmware(int version): m_comment(""), m_version(version), m_firmware("") {};

    /**
     * Constructor.
     */
    ARICHFirmware(const std::string& comment, int version, const std::string& firmware): m_comment(comment), m_version(version),
      m_firmware(firmware) {};

    /**
     * Destructor.
     */
    ~ARICHFirmware() {};

    /**
     * Get comment.
     * @return Comment.
     */
    std::string getFirmwareComment() const {return m_comment; }

    /**
     * Set comment.
     * @param[in] comment Comment.
     */
    void setFirmwareComment(const std::string& comment) {m_comment = comment; }

    /**
     * Get version number.
     * @return Version number.
     */
    unsigned int getFirmwareVersion() const {return m_version; }

    /**
     * Set version number.
     * @param[in] version Version number.
     */
    void setFirmwareVersion(unsigned int version) {m_version = version; }

    /**
     * Get FPGA bitfile.
     * @return FPGA bitfile.
     */
    std::string getFPGA() const {return m_firmware; }

    /**
     * Set FPGA bitfile.
     * @param[in] firmware FPGA bitfile.
     */
    void setFPGA(const std::string& firmware) {m_firmware = firmware; }


  private:
    std::string m_comment;         /**< comment */
    unsigned int m_version;        /**< version number */
    std::string m_firmware;        /**< FPGA bitfile */

    ClassDef(ARICHFirmware, 1);  /**< ClassDef */
  };
} // end namespace Belle2

