/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>
#include <vector>

#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for ARICH aerogel Rayleigh scattering fit parameters.
   */
  class ARICHAerogelRayleighScatteringFit: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHAerogelRayleighScatteringFit() : m_version(0.0), m_serial(""), m_comment(""), m_par() {};

    /**
     * Constructor.
     */
    ARICHAerogelRayleighScatteringFit(float version, const std::string& serial, const std::string& comment,
                                      const std::vector<float>& par) :
      m_version(version), m_serial(serial), m_comment(comment), m_par(par)
    {};

    /**
     * Destructor.
     */
    ~ARICHAerogelRayleighScatteringFit() {};

    /**
     * Get aerogel measurement version.
     * @return Aerogel measurement version.
     */
    float getAerogelMeasurementVersion() const { return m_version; }

    /**
     * Set aerogel measurement version.
     * @param[in] version Aerogel measurement version.
     */
    void setAerogelMeasurementVersion(float version) { m_version = version; }

    /**
     * Get aerogel serial number.
     * @return Aerogel serial number.
     */
    std::string getAerogelSerialNumber() const { return m_serial; };

    /**
     * Set aerogel serial number.
     * @param[in] serial Aerogel serial number.
     */
    void setAerogelSerialNumber(const std::string& serial) { m_serial = serial; }

    /**
     * Get comment.
     * @return Comment.
     */
    std::string getComment() const { return m_comment; }

    /**
     * Set comment (should be a single word).
     * @param[in] comment Comment.
     */
    void setComment(const std::string& comment) { m_comment = comment; }

    /**
     * Get aerogel Rayleigh scattering fit parameters.
     * @return Aerogel Rayleigh scattering fit parameters.
     */
    const std::vector<float>& getFitParameters() const { return m_par; }

    /**
     * Print the content of the class
     */
    void printContent(bool printHeader = false);

  private:

    float m_version;           /**< Aerogel data version */
    std::string m_serial;      /**< Aerogel Serial number */
    std::string m_comment;     /**< Optional comment (should be a single word)*/
    std::vector<float> m_par;  /**< Parameters value */

    ClassDef(ARICHAerogelRayleighScatteringFit, 1);  /**< ClassDef */
  };
} // end namespace Belle2
