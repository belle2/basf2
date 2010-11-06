/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorBase.h>

#ifndef GEOMATERIALS_H_
#define GEOMATERIALS_H_


namespace Belle2 {

  class GearDir;

  /**
   * The GeoMaterials class.
   *
   * The creator for the materials of the Belle II detector.
   */
  class GeoMaterials : public CreatorBase {

  public:

    /**
     * Constructor of the GeoMaterials class.
     */
    GeoMaterials();

    /**
     * The destructor of the GeoMaterials class.
     */
    virtual ~GeoMaterials();

    /** Creates the ROOT materials.
     *
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    virtual void create(GearDir& content);


  protected:

    int m_matIndex; /**< Each material gets an index assigned. Needed by ROOT. */

    /**
     * Create all single element materials.
     *
     * @param content A reference to the content part of the parameter description.
     */
    void createMaterialSingle(GearDir& content);

    /**
     * Create all material mixtures.
     *
     * @param content A reference to the content part of the parameter description.
     */
    void createMaterialMixture(GearDir& content);

  private:

    /**
     * Reads the values of a Material and returns a pointer to the created Material object.
     *
     * @param materialContent A GearDir pointing to the material description.
     * @param weight Returns a weight for a material mixture if it is defined. If not -1 is returned.
     * @return A pointer to the created TGeoMaterial object.
     */
    TGeoMaterial* readMaterial(GearDir& materialContent, double& weight);

    /**
     * Reads the values of an Element and returns a pointer to the created Element object.
     *
     * @param materialContent A GearDir pointing to the element description.
     * @param weight Returns a weight for a material mixture.
     * @return A pointer to the created TGeoElement object.
     */
    TGeoElement* readElement(GearDir& elementContent, double& weight);

  };
}

#endif /* GEOMATERIALS_H_ */
