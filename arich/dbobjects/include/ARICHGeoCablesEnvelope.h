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

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>

//root
#include <TVector3.h>

namespace Belle2 {

  /**
   * Geometry parameters of cable envelope
   */
  class ARICHGeoCablesEnvelope: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoCablesEnvelope()
    {}

    /**
     * Set Effective material name describing cables
     * @param materialName Effective material name describing cables
     */
    void setCablesEffectiveMaterialName(const std::string& materialName) {m_cablesEffectiveMaterialName = materialName;}

    /**
     * Set outer radius of cables envelop
     * @param envelopeOuterRadius outer radius of cables envelop
     */
    void setEnvelopeOuterRadius(double envelopeOuterRadius) {m_envelopeOuterRadius = envelopeOuterRadius;}

    /**
     * Set inner radius of cables envelop
     * @param envelopeInnerRadius inner radius of cables envelop
     */
    void setEnvelopeInnerRadius(double envelopeInnerRadius) {m_envelopeInnerRadius = envelopeInnerRadius;}

    /**
     * Set thickness of cables envelop
     * @param envelopeThickness thickness of cables envelop
     */
    void setEnvelopeThickness(double envelopeThickness) {m_envelopeThickness = envelopeThickness;}

    /**
     * Set position of cables envelop
     * @param x0 x position of cables envelop
     * @param y0 y position of cables envelop
     * @param z0 z position of cables envelop
     */
    void setEnvelopeCenterPosition(double x0, double y0, double z0) {m_envelopeX0 = x0; m_envelopeY0 = y0; m_envelopeZ0 = z0;}

    /**
     * Returns Effective material name describing cables
     * @return Effective material name describing cables
     */
    const std::string& getCablesEffectiveMaterialName() const {return m_cablesEffectiveMaterialName;}

    /**
     * Returns Outer radius of cables envelop
     * @return Outer radius of cables envelop
     */
    double getEnvelopeOuterRadius() const {return m_envelopeOuterRadius;}

    /**
     * Returns Inner radius of cables envelop
     * @return Inner radius of cables envelop
     */
    double getEnvelopeInnerRadius() const {return m_envelopeInnerRadius;}

    /**
     * Returns Thickness of cables envelop
     * @return Thickness of cables envelop
     */
    double getEnvelopeThickness() const {return m_envelopeThickness;}

    /**
     * Returns position vector (TVector3) of cables envelop
     * @return position vector (TVector3) of cables envelop
     */
    TVector3 getEnvelopeCenterPosition() const {return TVector3(m_envelopeX0, m_envelopeY0, m_envelopeZ0);}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Merger PCB geometry parameters") const;

    /**
     * Check data consistency of cables envelop
     * In case of failure print the BASF2 ERROR message using B2ASSERT
     */
    void checkCablesEnvelopDataConsistency() const;

  private:

    std::string m_cablesEffectiveMaterialName; /**< Effective material describing cables */
    double m_envelopeOuterRadius = 0.0;        /**< Outer radius of cables envelop */
    double m_envelopeInnerRadius = 0.0;        /**< Inner radius of cables envelop */
    double m_envelopeThickness = 0.0;          /**< Thickness of cables envelop */
    double m_envelopeX0 = 0.0;                 /**< X0 ARICH cables envelop center */
    double m_envelopeY0 = 0.0;                 /**< Y0 ARICH cables envelop center */
    double m_envelopeZ0 = 0.0;                 /**< Z0 ARICH cables envelop center */

    ClassDef(ARICHGeoCablesEnvelope, 1);       /**< ClassDef */

  };

} // end namespace Belle2
