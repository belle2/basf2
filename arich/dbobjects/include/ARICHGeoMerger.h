/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * Set single merger PCB and merger cooling envelop length
     * @param sizeL merger PCB and merger cooling envelop length
     */
    void setSingleMergerEnvelopeSizeL(double sizeL) {m_singlemergerenvelopeSizeL = sizeL;}

    /**
     * Set single merger PCB and merger cooling envelop width
     * @param sizeW merger PCB and merger cooling envelop width
     */
    void setSingleMergerEnvelopeSizeW(double sizeW) {m_singlemergerenvelopeSizeW = sizeW;}

    /**
     * Set single merger PCB and merger cooling envelop thickness
     * @param thickness single merger PCB and merger cooling envelop thickness
     */
    void setSingleMergerEnvelopeThickness(double thickness) {m_singlemergerenvelopeThickness = thickness;}

    /**
     * Set vector of Z position of the single merger and merger cooling body envelope inside global merger envelope in mm
     * @param singlemergerenvelopeDeltaZ vector of Z position of the single merger and merger cooling body envelope inside global merger envelope in mm
     */
    void setSingleMergerenvelopeDeltaZ(const std::vector<double>& singlemergerenvelopeDeltaZ) { m_singlemergerenvelopeDeltaZ = singlemergerenvelopeDeltaZ;}

    /**
     * Set merger PCB screw hole radius
     * @param screwholeR merger PCB screw hole radius
     */
    void setMergerPCBscrewholeR(double screwholeR) {m_mergerPCBscrewholeR = screwholeR;}

    /**
     * Set merger PCB screw hole position from the left and right sides
     * @param screwholePosdY merger PCB screw hole position from the left and right sides
     */
    void setMergerPCBscrewholePosdY(double screwholePosdY) { m_mergerPCBscrewholePosdY = screwholePosdY;}

    /**
     * Set merger PCB screw hole position from the bottom edge
     * @param screwholePosdX1 merger PCB screw hole position from the bottom edge
     */
    void setMergerPCBscrewholePosdX1(double screwholePosdX1) { m_mergerPCBscrewholePosdX1 = screwholePosdX1;}

    /**
     * Set merger PCB screw hole position from the top edge
     * @param screwholePosdX2 merger PCB screw hole position from the top edge
     */
    void setMergerPCBscrewholePosdX2(double screwholePosdX2) { m_mergerPCBscrewholePosdX2 = screwholePosdX2;}

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
     * Set position of merger PCB inside the single merger envelope
     * @param x0 x of merger PCB inside the single merger envelope
     * @param y0 y of merger PCB inside the single merger envelope
     * @param z0 z of merger PCB inside the single merger envelope
     */
    void setSingleMergeEnvelopePosition(double x0, double y0, double z0) {m_envelopePosX0 = x0; m_envelopePosY0 = y0; m_envelopePosZ0 = z0;}

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
     * Set vector of merger boarts orientations
     * @param mergerOrientation vector of merger boarts orientations
     */
    void setMergerOrientation(const std::vector<double>& mergerOrientation) { m_mergerOrientation = mergerOrientation;}

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
     * Returns single merger PCB and merger cooling envelop length
     * @return single merger PCB and merger cooling envelop length
     */
    double getSingleMergerEnvelopeSizeL() const {return m_singlemergerenvelopeSizeL;}

    /**
     * Returns single merger PCB and merger cooling envelop width
     * @return single merger PCB and merger cooling envelop width
     */
    double getSingleMergerEnvelopeSizeW() const {return m_singlemergerenvelopeSizeW;}

    /**
     * Returns single merger PCB and merger cooling envelop thickness
     * @return single merger PCB and merger cooling envelop thickness
     */
    double getSingleMergerEnvelopeThickness() const {return m_singlemergerenvelopeThickness;}

    /**
     * Returns vector of Z position of the single merger and merger cooling body envelope inside global merger envelope in mm
     * @return vector of Z position of the single merger and merger cooling body envelope inside global merger envelope in mm
     */
    const std::vector<double>& getSingleMergerenvelopeDeltaZ() const {return m_singlemergerenvelopeDeltaZ;}

    /**
     * Returns merger PCB screw hole radius
     * @return merger PCB screw hole radius
     */
    double getMergerPCBscrewholeR() const {return m_mergerPCBscrewholeR;}

    /**
     * Returns merger PCB screw hole position from the left and right sides
     * @return merger PCB screw hole position from the left and right sides
     */
    double getMergerPCBscrewholePosdY() const {return m_mergerPCBscrewholePosdY;}

    /**
     * Returns merger PCB screw hole position from the bottom edge
     * @return merger PCB screw hole position from the bottom edge
     */
    double getMergerPCBscrewholePosdX1() const {return m_mergerPCBscrewholePosdX1;}

    /**
     * Returns merger PCB screw hole position from the top edge
     * @return merger PCB screw hole position from the top edge
     */
    double getMergerPCBscrewholePosdX2() const {return m_mergerPCBscrewholePosdX2;}

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
     * Returns position vector (TVector3) of merger PCB inside the single merger envelope
     * @return position vector (TVector3) of merger PCB inside the single merger envelope
     */
    TVector3 getSingleMergeEnvelopePosition() const {return TVector3(m_envelopePosX0, m_envelopePosY0, m_envelopePosZ0);}

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
     * Returns vector of merger boarts orientations in deg
     * @return vector of merger boarts orientations in deg
     */
    const std::vector<double>& getMergerOrientation() const {return m_mergerOrientation;}

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
    double m_singlemergerenvelopeSizeL = 0.0;     /** Size of single merger PCB and merger cooling envelop : length */
    double m_singlemergerenvelopeSizeW = 0.0;     /** Size of single merger PCB and merger cooling envelop : width */
    double m_singlemergerenvelopeThickness = 0.0; /** Thickenss of single merger PCB and merger cooling envelop */
    /** Z position of the single merger and merger cooling body envelope inside global merger envelope in mm */
    std::vector<double> m_singlemergerenvelopeDeltaZ;
    double m_mergerPCBscrewholeR = 0.0;      /** Merger PCB screw hole radius */
    double m_mergerPCBscrewholePosdY = 0.0;  /** Merger PCB screw hole position from the left and right sides */
    double m_mergerPCBscrewholePosdX1 = 0.0; /** Merger PCB screw hole position from the bottom edge */
    double m_mergerPCBscrewholePosdX2 = 0.0; /** Merger PCB screw hole position from the top edge */
    double m_envelopeOuterRadius = 0.0;  /** Outer radius of merger PCB assembly envelope */
    double m_envelopeInnerRadius = 0.0;  /** Inner radius of merger PCB assembly envelope */
    double m_envelopeThickness = 0.0;    /** Thickness of merger PCB assembly envelope */
    double m_envelopeX0 = 0.0;           /** X0 merger PCB assembly envelope */
    double m_envelopeY0 = 0.0;           /** Y0 merger PCB assembly envelope */
    double m_envelopeZ0 = 0.0;           /** Z0 merger PCB assembly envelope */
    double m_envelopePosX0 = 0.0; /** Position of the merger inside the envelope */
    double m_envelopePosY0 = 0.0; /** Position of the merger inside the envelope */
    double m_envelopePosZ0 = 0.0; /** Position of the merger inside the envelope */
    std::vector<double> m_mergerSlotID;  /** Merger slot number */
    std::vector<double> m_mergerPosR;    /** Distance from the center in mm */
    std::vector<double> m_mergerAngle;   /** Azimuthal angle of the merger PCB center in polar coordinate system in deg */
    std::vector<double> m_mergerOrientation; /** Orientation of the merger board in deg */

    ClassDef(ARICHGeoMerger, 2);         /**< ClassDef */

  };

} // end namespace Belle2
