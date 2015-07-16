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
#include <string>
#include <cmath>
#include <framework/logging/Logger.h>
#include <vector>

namespace Belle2 {

  /**
   * The Class for ARICH Aerogel Parameters.
   */

  class ARICHAerogelInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHAerogelInfo(): m_version(0.0), m_refractiveIndex(0.0), m_transmissionLength(0), m_thickness(0.0)
    {
      for (int i = 0; i < 26; i++) m_lambda[i] = 200 + i * 20;
      for (int i = 0; i < 26; i++) m_transmittance[i] = 0;
    };

    /**
     * Constructor
     */
    ARICHAerogelInfo(double version, std::string serial, std::string id, double index, double trlen, double thickness,
                     std::vector<int> lambda, std::vector<double> transmittance)
    {
      m_version = version;
      m_serial = serial;
      m_id = id;
      m_refractiveIndex = index;
      m_transmissionLength = trlen;
      m_thickness = thickness;
      m_lambda = lambda;
      m_transmittance = transmittance;
    }

    /**
     * Constructor
     */
    ARICHAerogelInfo(double version, std::string serial, std::string id, double index, double trlen, double thickness)
    {
      m_version = version;
      m_serial = serial;
      m_id = id;
      m_refractiveIndex = index;
      m_transmissionLength = trlen;
      m_thickness = thickness;
    }

    /**
     * Constructor
     */
    ARICHAerogelInfo(double refractiveIndex, double transmissionLength)
    {
      m_refractiveIndex = refractiveIndex;
      m_transmissionLength = transmissionLength;
    }

    /**
     * Destructor
     */
    ~ARICHAerogelInfo() {};



    /** Return Aerogel Version
     * @return Aerogel Version
     */

    double getAerogelVersion() const {return m_version; }

    /** Set Aerogel Version
     * @param Aerogel Version
     */
    void setAerogelVersion(double version) {m_version = version; }

    /** Return Aerogel Serial number
     * @return Aerogel Serial number
     */
    std::string getAerogelSerial();

    /** Set Aerogel Serial number
     * @param Aerogel Serial number
     */
    void setAerogelSerial(const std::string& serial) {m_serial = serial; }

    /** Return Aerogel Identifier
     * @return Aerogel Identifier
     */
    std::string getAerogelID();

    /** Set Aerogel Identifier
     * @param Aerogel Identifier
     */
    void setAerogelID(const std::string& agelid) {m_id = agelid; }

    /** Return Refractive Index of Aerogel
     * @return Refractive Index of Aerogel
     */

    double getAerogelRefractiveIndex() const {return m_refractiveIndex; }

    /** Set Aerogel Refractive Index
     * @param Aerogel Refractive Index
     */
    void setAerogelRefractiveIndex(double index) {m_refractiveIndex = index; }

    /** Return Transmission Length at 400 nm
     * @return Transmission Length at 400 nm
     */
    double getAerogelTransmissionLength() const {return m_transmissionLength; }

    /** Set Transmission Length at 400 nm
     * @param Transmission Length at 400 nm
     */
    void setAerogelTransmissionLength(double translength) {m_transmissionLength = translength; }

    /** Return Aerogel Thickness
     * @return Aerogel Thickness
     */

    double getAerogelThickness() const {return m_thickness; }

    /** Set Aerogel Thickness
     * @param Aerogel Thickness
     */
    void setAerogelThickness(double thickness) {m_thickness = thickness; }



    /**
     * Return Aerogel Transmittance
     * @return Aerogel transmittance
     */

    double getAerogelTransmittanceElement(int lambda) const
    {
      if (lambda < 200 || lambda > 700) {
        B2INFO("lambda not in right interval!");
        return -1;
      }
      int index = (int) round((lambda - 200.0) / 20.0);
      return m_transmittance[index];
    }

    /**
     * Set Aerogel Transmittance at lambda
     * @param lambda
     * @param transmittance
     */
    void setAerogelTransmittanceElement(int lambda, double transmittance)
    {
      if (lambda < 200 || lambda > 700) {
        B2INFO("lambda not in right interval!");
      }
      int index = (int) round((lambda - 200.0) / 20.0);
      m_transmittance[index] = transmittance;
    }


    /**
     * Set Aerogel Transmittance
     * @param transmittance
     */
    void setAerogelTransmittance(std::vector<double> transmittance)
    {
      for (unsigned int i = 0; i < transmittance.size(); i++)  m_transmittance.push_back(transmittance[i]);
    }

    /** Return comment
     * @return comment
     */
    std::string getAerogelComment();

    /** Set comment
     * @param comment
     */
    void setAerogelComment(const std::string& comment) {m_comment = comment; }

  private:

    double m_version;                          /**< Aerogel version */
    std::string m_serial;                      /**< Aerogel Serial number */
    std::string m_id;                          /**< Aerogel Identifier */
    double m_refractiveIndex;                  /**< Refractive Index of the aerogel */
    double m_transmissionLength;               /**< Transmission Length at 400 nm */
    double m_thickness;                        /**< Thickness of the aerogel */
    std::vector<int> m_lambda;                 /**< Lambda for transmittance */
    std::vector<double> m_transmittance;       /**< Transmittance of aerogel */
    std::string m_comment;                     /**< optional comment */


    ClassDef(ARICHAerogelInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2




