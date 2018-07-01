/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leonid Burmistrov                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>
#include <cmath>
#include <vector>

#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for ARICH aerogel Rayleigh scattering fit parameters.
   */
  class ARICHAerogelRayleighScatteringFit: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHAerogelRayleighScatteringFit() : m_version(0.0), m_serial(""), m_comment(""), m_par() {};

    /**
     * Constructor
     */
    ARICHAerogelRayleighScatteringFit(float version, const std::string& serial, const std::string& comment, std::vector<float>& par) :
      m_version(version), m_serial(serial), m_comment(comment), m_par(par)
    {};

    /**
     * Destructor
     */
    ~ARICHAerogelRayleighScatteringFit() {};

    /** Return aerogel measurement version
     * @return aerogel measurement version
     */
    float getAerogelMeasurementVersion() const { return m_version; }

    /** Set aerogel measurement version
     * @param aerogel measurement version
     */
    void setAerogelMeasurementVersion(float version) { m_version = version; }

    /** Return aerogel serial number
     * @return aerogel serial number
     */
    std::string getAerogelSerialNumber() const { return m_serial; };

    /** Set aerogel serial number
     * @param aerogel serial number
     */
    void setAerogelSerialNumber(const std::string& serial) { m_serial = serial; }

    /** Set comment (should be a single word)
     * @param comment
     */
    void setComment(const std::string& comment) { m_comment = comment; }


    /** Return comment
     * @return comment
     */
    std::string getComment() const { return m_comment; }


    /**
     * Return aerogel Rayleigh scattering fit parameters
     * @return aerogel Rayleigh scattering fit parameters
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
