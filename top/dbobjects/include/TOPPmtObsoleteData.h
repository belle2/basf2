/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Alessandro Gaz                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>

namespace Belle2 {

  /**
   * PMT specs from Hamamatsu
   *
   * These are provided only for book-keeping purposes and these data
   * should never be used in the actual detector
   */
  class TOPPmtObsoleteData : public TObject {
  public:
    /**
     * enum for PMT types
     */
    enum EType {c_Unknown = 0, c_Conventional, c_ALD, c_LifeExtALD};

    /**
     * Default constructor
     */
    TOPPmtObsoleteData()
    {}

    /**
     * Full constructor
     * @param serialNumber serial number
     * @param type PMT type
     * @param cathode phote-cathode type
     * @param HV obsolete high voltage in V
     * @param darkCurrent dark current in Hz
     * @param qe380 quantum efficiency at 380 nm
     */
    TOPPmtObsoleteData(const std::string& serialNumber,
                       EType type,
                       const std::string& cathode,
                       float obsHV,
                       float darkCurrent,
                       float qe380):
      m_serialNumber(serialNumber), m_type(type), m_cathode(cathode),
      m_obsoleteHV(obsHV), m_darkCurrent(darkCurrent), m_qe380(qe380)
    {}

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns PMT type
     * @return type
     */
    EType getType() const {return m_type;}

    /**
     * Returns photo-cathode type
     * @return photo-cathode type
     */
    const std::string& getCathode() const {return m_cathode;}

    /**
     * Returns high voltage
     * @return high voltage in V
     */
    float getObsoleteHV() const {return m_obsoleteHV;}

    /**
     * Returns dark current
     * @return dark current in Hz
     */
    float getDarkCurrent() const {return m_darkCurrent;}

    /**
     * Returns quantum efficiency at 380 nm
     * @return quantum efficiency
     */
    float getQE() const {return m_qe380;}

  private:

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    EType m_type = c_Unknown;   /**< type (conventional, ALD, ...) */
    std::string m_cathode;      /**< photo-cathode type (e.g. NaKSb, bialkali etc) */
    float m_obsoleteHV = 0;     /**< high voltage: obsolete - NOT to be used */
    float m_darkCurrent = 0;    /**< dark current */
    float m_qe380 = 0;          /**< quantum efficiency at 380 nm */

    ClassDef(TOPPmtObsoleteData, 2); /**< ClassDef */

  };

} // end namespace Belle2


