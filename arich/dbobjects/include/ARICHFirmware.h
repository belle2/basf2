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
  *  ARICH firmware
  */
  class ARICHFirmware: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHFirmware(): m_comment(""), m_version(0), m_firmware("") {};

    /**
     * Constructor
     */
    explicit ARICHFirmware(int version): m_comment(""), m_version(version), m_firmware("") {};

    /**
     * Constructor
     */
    ARICHFirmware(const std::string& comment, int version, const std::string& firmware): m_comment(comment), m_version(version),
      m_firmware(firmware) {};

    /**
     * Destructor
     */
    ~ARICHFirmware() {};

    /** Return comment
     * @return comment
     */
    std::string getFirmwareComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setFirmwareComment(const std::string& comment) {m_comment = comment; }

    /** Return Version Number
     * @return Version Number
     */
    unsigned int getFirmwareVersion() const {return m_version; }

    /** Set Version Number
     * @param Version Number
     */
    void setFirmwareVersion(unsigned int version) {m_version = version; }

    /** Return FPGA bitfile
     * @return FPGA bitfile
     */
    std::string getFPGA() const {return m_firmware; }

    /** Set FPGA bitfile
     * @param FPGA bitfile
     */
    void setFPGA(const std::string& firmware) {m_firmware = firmware; }


  private:
    std::string m_comment;         /**< comment */
    unsigned int m_version;        /**< version number */
    std::string m_firmware;        /**< FPGA bitfile */

    ClassDef(ARICHFirmware, 1);  /**< ClassDef */
  };
} // end namespace Belle2

