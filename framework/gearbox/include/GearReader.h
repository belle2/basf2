/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARREADER_H_
#define GEARREADER_H_

#include <TGeoMaterial.h>

namespace Belle2 {

  class GearDir;

  /**
   * The GearReader class.
   *
   * Provides static methods to read special objects
   * from a GearDir.
   */
  class GearReader {

  public:

    /**
     * Reads a material definition section and returns a pointer to the created TGeoMaterial object.
     * The material section can be either a <Material> or a <Mixture>.
     *
     * The following definitions for a <Material> are supported:
     * <Material name="">
     *   <Density unit="g/cm3"></Density>
     *   <RadLength></RadLength>
     *   <InterLength></InterLength>
     *   <AtomNumber> </AtomNumber>
     *   <MassNumber> </MassNumber>
     * </Material>
     *
     * <Material name="">
     *   <Density unit="g/cm3"></Density>
     *   <AtomNumber> </AtomNumber>
     *   <MassNumber> </MassNumber>
     * </Material>
     *
     * A Mixture consist of elements and a weight factor for each of them.
     * These elements can be an arbitrary combination of Elements, Materials
     * or other Mixtures. The basic structure is:
     * <Mixture name="">
     *   <Density unit="g/cm3"></Density>
     *   <Elements>
     *     add here the elements of the mixture.
     *   </Elements>
     * </Mixture>
     *
     * The elements of a mixture can be
     * 1) Elements
     *    <Element name="" weight="">
     *      <AtomNumber></AtomNumber>
     *      <MassNumber></MassNumber>
     *    </Element>
     *
     * 2) Materials
     *    Both material definitions given above are supported.
     *
     * 3) Mixtures
     *    As described here.
     *
     * 4) Existing Materials or Mixtures
     *    <Material name="" weight=""/>
     *    or
     *    <Mixture name="" weight=""/>
     *
     * @param gearDir Reference to a GearDir pointing to the TGeoMaterial or TGeoMixture definition.
     */
    static TGeoMaterial* readMaterial(GearDir& gearDir);


  protected:

  private:

  };

}

#endif /* GEARREADER_H_ */
