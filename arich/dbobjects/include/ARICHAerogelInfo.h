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
   * The Class for ARICH Aerogel Parameters.
   */
  class ARICHAerogelInfo: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHAerogelInfo(): m_version(0.0), m_serial(""), m_id(""), m_refractiveIndex(0.0), m_transmissionLength(0), m_thickness(0.0),
      m_lambda(), m_transmittance(), m_comment("") {};

    /**
     * Constructor.
     */
    ARICHAerogelInfo(float version, const std::string& serial, const std::string& id, float index, float trlen, float thickness,
                     std::vector<int>& lambda, std::vector<float>& transmittance): m_version(version), m_serial(serial), m_id(id),
      m_refractiveIndex(index), m_transmissionLength(trlen), m_thickness(thickness), m_lambda(lambda), m_transmittance(transmittance),
      m_comment("") {};

    /**
     * Destructor.
     */
    ~ARICHAerogelInfo() {};

    /**
     * Get aerogel version.
     * @return aerogel version.
     */
    float getAerogelVersion() const {return m_version; }

    /**
     * Set aerogel version.
     * @param[in] version Aerogel version.
     */
    void setAerogelVersion(float version) {m_version = version; }

    /**
     * Get aerogel serial number.
     * @return Aerogel serial number.
     */
    std::string getAerogelSN() const {return m_serial; };

    /**
     * Set aerogel serial number.
     * @param[in] serial Aerogel serial number.
     */
    void setAerogelSN(const std::string& serial) {m_serial = serial; }

    /**
     * Get aerogel identifier.
     * @return Aerogel identifier.
     */
    std::string getAerogelID() const {return m_id; };

    /**
     * Set aerogel identifier.
     * @param[in] agelid Aerogel identifier.
     */
    void setAerogelID(const std::string& agelid) {m_id = agelid; }

    /**
     * Get refractive index of aerogel.
     * @return Refractive index of aerogel.
     */
    float getAerogelRefractiveIndex() const {return m_refractiveIndex; }

    /**
     * Set aerogel refractive index.
     * @param[in] index Aerogel refractive index.
     */
    void setAerogelRefractiveIndex(float index) {m_refractiveIndex = index; }

    /**
     * Get transmission length at 400 nm.
     * @return Transmission length at 400 nm.
     */
    float getAerogelTransmissionLength() const {return m_transmissionLength; }

    /**
     * Set transmission length at 400 nm.
     * @param[in] translength Transmission Length at 400 nm.
     */
    void setAerogelTransmissionLength(float translength) {m_transmissionLength = translength; }

    /**
     * Get aerogel thickness.
     * @return Aerogel thickness.
     */
    float getAerogelThickness() const {return m_thickness; }

    /**
     * Set aerogel thickness.
     * @param[in] thickness Aerogel thickness.
     */
    void setAerogelThickness(float thickness) {m_thickness = thickness; }

    /**
     * Get aerogel transmittance at lambda.
     * @return Aerogel transmittance at lambda.
     */
    float getAerogelTransmittanceElement(int lambda) const;

    /**
     * Set aerogel transmittance at lambda.
     * @param[in] lambda
     * @param[in] transmittance Aerogel transmittance at lambda.
     */
    void setAerogelTransmittanceElement(int lambda, float transmittance);

    /**
     * Get aerogel transmittance
     * @return Aerogel transmittance.
     */
    float getAerogelTransmittance(unsigned int i) const { if (i < m_transmittance.size()) return m_transmittance[i]; else return -1;}

    /**
     * Set aerogel transmittance.
     * @param[in] transmittance Aerogel transmittance.
     */
    void setAerogelTransmittance(std::vector<float> transmittance);

    /**
     * Get lambda for aerogel transmittance.
     * @return Lambda for aerogel transmittance.
     */
    int getAerogelTransmittanceLambda(unsigned int i) const { if (i < m_lambda.size()) return m_lambda[i]; else return -1;}

    /**
     * Set lambda for aerogel transmittance.
     * @param[in] lambda Lambda for aerogel transmittance.
     */
    void setAerogelTransmittanceLambda(std::vector<float> lambda);

    /**
     * Get comment.
     * @return Comment.
     */
    std::string getAerogelComment() const {return m_comment; }

    /**
     * Set comment.
     * @param[in] comment Comment.
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
