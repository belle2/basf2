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


  private:

    /** Creates all media in the GearDir 'content' having the node name given by nodeName.
     *
     * @param content A reference to the content part which contains the description of the media.
     * @param content The name of the nodes for which TGeoMedium objects should be created.
     */
    void createMedia(GearDir& content, const std::string nodeName);

  };
}

#endif /* GEOMATERIALS_H_ */
