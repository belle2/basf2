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
   * Geometry parameters of Merger PCB
   */
  class ARICHGeoMerger: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoMerger()
    {}

    /**
     * Set merger PCB material name
     * @param materialName merger PCB material name
     */
    void setMergerPCBMaterialName(const std::string& materialName) {m_mergerPCBMaterialName = materialName;}

    /**
     * Set merger PCB lenght
     * @param sizeL merger PCB lenght
     */
    void setMergerPCBLenght(double sizeL) {m_sizeL = sizeL;}

    /**
     * Set merger PCB width
     * @param sizeW merger PCB width
     */
    void setMergerPCBWidth(double sizeW) {m_sizeW = sizeW;}

    /**
     * Set merger PCB thickness
     * @param thickness merger PCB thickness
     */
    void setMergerPCBThickness(double thickness) {m_thickness = thickness;}

    /**
     * Set outer radius of merger PCB assembly envelope
     * @param envelopeOuterRadius outer radius of merger PCB assembly envelope
     */
    void setEnvelopeOuterRadius(double envelopeOuterRadius) {m_envelopeOuterRadius = envelopeOuterRadius;}

    /**
     * Set inner radius of merger PCB assembly envelope
     * @param envelopeInnerRadius inner radius of merger PCB assembly envelope
     */
    void setEnvelopeInnerRadius(double envelopeInnerRadius) {m_envelopeInnerRadius = envelopeInnerRadius;}

    /**
     * Set thickness of merger PCB assembly envelope
     * @param envelopeThickness thickness of merger PCB assembly envelope
     */
    void setEnvelopeThickness(double envelopeThickness) {m_envelopeThickness = envelopeThickness;}

    /**
     * Set position of merger PCB assembly envelope
     * @param x0 x position of merger PCB assembly envelope
     * @param y0 y position of merger PCB assembly envelope
     * @param z0 z position of merger PCB assembly envelope
     */
    void setEnvelopeCenterPosition(double x0, double y0, double z0) {m_envelopeX0 = x0; m_envelopeY0 = y0; m_envelopeZ0 = z0;}

    /**
     * Set vector of merger boards slot numbers
     * @param mergerSlotID vector of merger slot numbers
     */
    void setMergerSlotID(const std::vector<double>& mergerSlotID) { m_mergerSlotID = mergerSlotID;}

    /**
     * Set vector of merger boards distances from the center in mm
     * @param mergerPosR vector of merger boards distances from the center in mm
     */
    void setMergerPosR(const std::vector<double>& mergerPosR) { m_mergerPosR = mergerPosR;}

    /**
     * Set vector of merger boarts azimuthal angles in polar coordinate system in deg
     * @param mergerAngle vector of merger boarts azimuthal angles
     */
    void setMergerAngle(const std::vector<double>& mergerAngle) { m_mergerAngle = mergerAngle;}

    /**
     * Returns merger PCB material name
     * @return merger PCB material name
     */
    const std::string& getMergerPCBMaterialName() const {return m_mergerPCBMaterialName;}

    /**
     * Returns merger PCB lenght
     * @return merger PCB lenght
     */
    double getSizeL() const {return m_sizeL;}

    /**
     * Returns merger PCB width
     * @return merger PCB width
     */
    double getSizeW() const {return m_sizeW;}

    /**
     * Returns merger PCB thickness
     * @return merger PCB thickness
     */
    double getThickness() const {return m_thickness;}

    /**
     * Returns Outer radius of merger PCB assembly envelope
     * @return Outer radius of merger PCB assembly envelope
     */
    double getEnvelopeOuterRadius() const {return m_envelopeOuterRadius;}

    /**
     * Returns Inner radius of merger PCB assembly envelope
     * @return Inner radius of merger PCB assembly envelope
     */
    double getEnvelopeInnerRadius() const {return m_envelopeInnerRadius;}

    /**
     * Returns Thickness of merger PCB assembly envelope
     * @return Thickness of merger PCB assembly envelope
     */
    double getEnvelopeThickness() const {return m_envelopeThickness;}

    /**
     * Returns position vector (TVector3) of merger PCB assembly envelope
     * @return position vector (TVector3) of merger PCB assembly envelope
     */
    TVector3 getEnvelopeCenterPosition() const {return TVector3(m_envelopeX0, m_envelopeY0, m_envelopeZ0);}

    /**
     * Returns vector of merger boards slot numbers
     * @return vector of merger boards slot numbers
     */
    const std::vector<double>& getMergerSlotID() const {return m_mergerSlotID;}

    /**
     * Returns vector of merger boards distances from the center in mm
     * @return vector of merger boards distances from the center in mm
     */
    const std::vector<double>& getMergerPosR() const {return m_mergerPosR;}

    /**
     * Returns vector of merger boarts azimuthal angles in polar coordinate system in deg
     * @return vector of merger boarts azimuthal angles in polar coordinate system in deg
     */
    const std::vector<double>& getMergerAngle() const {return m_mergerAngle;}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Merger PCB geometry parameters") const;

    /**
     * Check data consistency of the merger positions
     * In case of failure print the BASF2 ERROR message using B2ASSERT
     */
    void checkMergerPositionsDataConsistency() const;

  private:

    std::string m_mergerPCBMaterialName; /**< Material of merger PCB */
    double m_sizeL = 0.0;                /**< Size of merger PCB : length */
    double m_sizeW = 0.0;                /**< Size of merger PCB : width */
    double m_thickness = 0.0;            /**< Thickenss of merger PCB */
    double m_envelopeOuterRadius = 0.0;  /**< Outer radius of merger PCB assembly envelope */
    double m_envelopeInnerRadius = 0.0;  /**< Inner radius of merger PCB assembly envelope */
    double m_envelopeThickness = 0.0;    /**< Thickness of merger PCB assembly envelope */
    double m_envelopeX0 = 0.0;           /**< X0 merger PCB assembly envelope */
    double m_envelopeY0 = 0.0;           /**< Y0 merger PCB assembly envelope */
    double m_envelopeZ0 = 0.0;           /**< Z0 merger PCB assembly envelope */
    std::vector<double> m_mergerSlotID;  /**< Merger slot number */
    std::vector<double> m_mergerPosR;    /**< Distance from the center in mm */
    std::vector<double> m_mergerAngle;   /**< Azimuthal angle of the merger PCB center in polar coordinate system in deg */

    ClassDef(ARICHGeoMerger, 1);         /**< ClassDef */

  };

} // end namespace Belle2
