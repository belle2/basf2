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
#include <vector>

#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for ARICH Aerogel Parameters.
   */

  class ARICHAerogelInfo: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHAerogelInfo(): m_version(0.0), m_serial(""), m_id(""), m_refractiveIndex(0.0), m_transmissionLength(0), m_thickness(0.0),
      m_lambda(), m_transmittance(), m_comment("") {};

    /**
     * Constructor
     */
    ARICHAerogelInfo(float version, const std::string& serial, const std::string& id, float index, float trlen, float thickness,
                     std::vector<int>& lambda, std::vector<float>& transmittance): m_version(version), m_serial(serial), m_id(id),
      m_refractiveIndex(index), m_transmissionLength(trlen), m_thickness(thickness), m_lambda(lambda), m_transmittance(transmittance),
      m_comment("") {};

    /**
     * Destructor
     */
    ~ARICHAerogelInfo() {};

    /** Return Aerogel Version
     * @return Aerogel Version
     */

    float getAerogelVersion() const {return m_version; }

    /** Set Aerogel Version
     * @param Aerogel Version
     */
    void setAerogelVersion(float version) {m_version = version; }

    /** Return Aerogel Serial number
     * @return Aerogel Serial number
     */
    std::string getAerogelSN() const {return m_serial; };

    /** Set Aerogel Serial number
     * @param Aerogel Serial number
     */
    void setAerogelSN(const std::string& serial) {m_serial = serial; }

    /** Return Aerogel Identifier
     * @return Aerogel Identifier
     */
    std::string getAerogelID() const {return m_id; };

    /** Set Aerogel Identifier
     * @param Aerogel Identifier
     */
    void setAerogelID(const std::string& agelid) {m_id = agelid; }

    /** Return Refractive Index of Aerogel
     * @return Refractive Index of Aerogel
     */

    float getAerogelRefractiveIndex() const {return m_refractiveIndex; }

    /** Set Aerogel Refractive Index
     * @param Aerogel Refractive Index
     */
    void setAerogelRefractiveIndex(float index) {m_refractiveIndex = index; }

    /** Return Transmission Length at 400 nm
     * @return Transmission Length at 400 nm
     */
    float getAerogelTransmissionLength() const {return m_transmissionLength; }

    /** Set Transmission Length at 400 nm
     * @param Transmission Length at 400 nm
     */
    void setAerogelTransmissionLength(float translength) {m_transmissionLength = translength; }

    /** Return Aerogel Thickness
     * @return Aerogel Thickness
     */
    float getAerogelThickness() const {return m_thickness; }

    /** Set Aerogel Thickness
     * @param Aerogel Thickness
     */
    void setAerogelThickness(float thickness) {m_thickness = thickness; }

    /**
     * Return Aerogel Transmittance at lambda
     * @return Aerogel transmittance at lambda
     */
    float getAerogelTransmittanceElement(int lambda) const;

    /**
     * Set Aerogel Transmittance at lambda
     * @param lambda
     * @param transmittance
     */
    void setAerogelTransmittanceElement(int lambda, float transmittance);
    /**
     * Return Aerogel Transmittance
     * @return Aerogel transmittance
     */
    float getAerogelTransmittance(unsigned int i) const { if (i < m_transmittance.size()) return m_transmittance[i]; else return -1;}

    /**
     * Set Aerogel Transmittance
     * @param transmittance
     */
    void setAerogelTransmittance(std::vector<float> transmittance);

    /**
     * Return Lambda for Aerogel Transmittance
     * @return Lambda for Aerogel transmittance
     */
    int getAerogelTransmittanceLambda(unsigned int i) const { if (i < m_lambda.size()) return m_lambda[i]; else return -1;}

    /**
     * Set Lambda for Aerogel Transmittance
     * @param Lambda for transmittance
     */
    void setAerogelTransmittanceLambda(std::vector<float> lambda);
    /** Return comment
     * @return comment
     */
    std::string getAerogelComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setAerogelComment(const std::string& comment) {m_comment = comment; }

  private:

    float c_lambdaMin = 200.0;            /**< Min value of lambda */
    float c_lambdaMax = 700.0;            /**< Max value of lambda */
    float c_lambdaStep = 10.0;            /**< Lambda - steps */

    float m_version;                      /**< Aerogel data version (measurement number) */
    std::string m_serial;                 /**< Aerogel Serial number */
    std::string m_id;                     /**< Aerogel Identifier */
    float m_refractiveIndex;              /**< Refractive Index of the aerogel */
    float m_transmissionLength;           /**< Transmission Length at 400 nm */
    float m_thickness;                    /**< Thickness of the aerogel */
    std::vector<int> m_lambda;            /**< Lambda for transmittance */
    std::vector<float> m_transmittance;   /**< Transmittance of aerogel */
    std::string m_comment;                /**< optional comment */


    ClassDef(ARICHAerogelInfo, 4);  /**< ClassDef */
  };
} // end namespace Belle2
