/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>

namespace Belle2 {
  /**
  *  ARICH firmware
  */
  class ARICHFirmware: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHFirmware(): m_version(0) {};

    /**
     * Constructor
     */
    ARICHFirmware(int version)
    {
      m_version = version;
    }

    /**
     * Destructor
     */
    ~ARICHFirmware() {};

    /** Return comment
     * @return comment
     */
    std::string getFirmwareComment();

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
    std::string getFPGA();

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

