/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Timofey Uglov                                           *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#ifndef EKLMTTRANSFORMATIONFACTORY_H_
#define EKLMTTRANSFORMATIONFACTORY_H_

#include <G4Transform3D.hh>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <eklm/dataobjects/EKLMHitBase.h>

namespace Belle2 {

  /**
   * Strip transformations.
   */
  class EKLMTransformationFactory {

  public:

    /**
     * Get instance.
     * @return EKLMTransformationFactory*.
     */
    static EKLMTransformationFactory* getInstance();

    /**
     * Add strip transformation matrix entry.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @param[in] strip  Strip number.
     * @param[in] matrix G4Transform3D.
     */
    void addMatrixEntry(int endcap, int layer, int sector, int plane,
                        int strip, G4Transform3D matrix);

    /**
     * Add strip length entry.
     * @param[in] strip  Strip number.
     * @param[in] length Length.
     */
    void addLengthEntry(int strip, double length);

    /**
     * Get transformation matrix for the strip.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @param[in] strip  Strip number.
     * @return Transformation.
     */
    G4Transform3D getTransformation(int endcap, int layer, int sector,
                                    int plane, int strip);

    /**
     * Get transformation matrix for the strip.
     * @param[in] id Strip identifier.
     * @return Transformation.
     */
    G4Transform3D getTransformation(EKLMStripID id);

    /**
     * Get transformation matrix for the strip.
     * where strip ID is taken from the parent class via virtual function.
     * @return Transformation.
     */
    G4Transform3D getTransformation(const EKLMHitBase* hit);

    /**
     * Get strip length.
     * @return Length of the the strip.
     */
    double getStripLength(int);

    /**
     * Get strip length.
     * @return Length of the the strip.
     */
    double getStripLength(const EKLMHitBase* hit);

    /**
     * Get strip length.
     * @return Length of the the strip.
     */
    double getStripLength(EKLMStripID id);

    /**
     * Reads information from XML file.
     * @param[in] gd XML data directory.
     */
    void readFromXMLFile(const GearDir& gd);

    /**
     * Read information from file to memory.
     * @param[in] filename Name of file.
     */
    void readFromFile(const char* filename);

    /**
     * Write collected information to file.'
     * @param[in] filename Name of file.
     */
    void writeToFile(const char* filename) const;

    /**
     * Write collected information to XML file.
     * @param[in] filename Name of file.
     */
    void writeToXMLFile(char* filename);

    /**
     * Clear.
     */
    void clear();

  private:

    /** Instance. */
    static EKLMTransformationFactory* EKLMTransformationFactory_instance;

    /**
     * Constructor.
     */
    EKLMTransformationFactory();

    /**
     * G4Transform3D matrix for the strip.
     * @details
     * [#endcap][#layer][#sector][#plane][#strip]
     */
    G4Transform3D stripMatrixArray[2][14][4][2][75];

    /** The strip length. */
    double stripLengthArray[75];

  };

}

#endif

