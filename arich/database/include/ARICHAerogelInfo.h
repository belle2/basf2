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

namespace Belle2 {

  /**
   * The Class for ARICH Aerogel Parameters.
   */

  class ARICHAerogelInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHAerogelInfo(): m_refractiveIndex(0), m_transmissionLength(0) {};

    /**
     * Default constructor
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

    /** Return Aerogel Identifier
     * @return Aerogel Identifier
     */
    std::string getAerogelID();

    /** Set Aerogel Identifier
     * @param Aerogel Identifier
     */
    void setAerogelIdentifier(const std::string& agelid) {m_id = agelid; }

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

    /** Return comment
     * @return comment
     */
    std::string getAerogelComment();

    /** Set comment
     * @param comment
     */
    void setAerogelComment(const std::string& comment) {m_comment = comment; }

  private:

    std::string m_id;               /**< Aerogel Identifier */
    double m_refractiveIndex;       /**< Refractive Index of the aerogel */
    double m_transmissionLength;    /**< Transmission Length at 400 nm */
    std::string m_comment;          /**< optional comment */

    ClassDef(ARICHAerogelInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2

