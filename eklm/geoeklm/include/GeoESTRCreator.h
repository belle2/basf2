/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOESTRCREATOR_H
#define GEOESTRCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>

/**
 * @file
 * ESTR geometry.
 */

namespace Belle2 {

  namespace ESTR {

    /**
     * ESTR geometry parameters.
     *
     * @var GeometryParams::phi
     * The starting angle of the octagonal Endcap KLM shape.
     *
     * @var GeometryParams::dphi
     * The opening angle (shape is extended from phi to phi+dphi).
     *
     * @var GeometryParams::nsides
     * The number of sides (=8 : octagonal).
     *
     * @var GeometryParams::nboundary
     * The number of boundaries perpendicular to the z-axis.
     *
     * @var GeometryParams::zsub
     * The length of the tube.
     *
     * @var GeometryParams::rminsub
     * The inner radius of the tube.
     *
     * @var GeometryParams::rmaxsub
     * The outer radius of the tube.
     *
     * @var GeometryParams::thick_eiron
     * The thickness of the mother vessel of the Endcap KLM module slot.
     *
     * @var GeometryParams::thick_eiron_meas
     * Measured thickness of the mother vessel of the Endcap KLM module slot.
     *
     * @var GeometryParams::thick_eslot
     * The thickness of the Endcap KLM module slot.
     *
     * @var GeometryParams::thick_eslot_meas
     * Measured thickness of the Endcap KLM module slot.
     *
     * @var GeometryParams::thick_emod
     * The thickness of the Endcap KLM module.
     *
     * @var GeometryParams::rmin_emod
     * The minimum radius of the Endcap KLM module.
     *
     * @var GeometryParams::rmax_emod
     * The maximum radius of the Endcap KLM module.
     *
     * @var GeometryParams::rmin_eslot
     * The minimum radius of the Endcap KLM module slot.
     *
     * @var GeometryParams::rmax_eslot
     * The maximum radius of the Endcap KLM module slot.
     *
     * @var GeometryParams::rshift_eslot
     * The shift value in the radial direction of the Endcap KLM module slot.
     *
     * @var GeometryParams::rmax_glass
     * The maximum radius of the glass used in KLM module (not used now).
     */
    struct GeometryParams {
      double phi;
      double dphi;
      int nsides;
      int nboundary;
      double* z;
      double* rmin;
      double* rmax;
      int zsub;
      double rminsub;
      double rmaxsub;
      double thick_eiron;
      double thick_eiron_meas;
      double thick_eslot;
      double thick_eslot_meas;
      double thick_emod;
      double rmin_emod;
      double rmax_emod;
      double rmin_eslot;
      double rmax_eslot;
      double rshift_eslot;
      double rmax_glass;
    };

    /**
     * Read ESTR geometry parameters.
     * @param par Geometry parameters.
     * @return 0      Successful.
     * @return ENOMEM Memory allocation error.
     * @details
     * Allocates memory for z, rmin and rmax arrays.
     */
    int readESTRData(struct GeometryParams* par);

    /**
     * Creator of the Belle II Endcap Structure.
     */
    class GeoESTRCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor.
       */
      GeoESTRCreator();

      /**
       * Destructor.
       */
      ~GeoESTRCreator();

      /**
       * Create ESTR geometry.
       * @param content   GearDir pointing to the parameters which
       *                  should be used for construction.
       * @param topVolume Top volume in which the geometry has to be placed.
       * @param type      Type of geometry to be build.
       * @details
       * ESTR geometry is actually created in EKLM geometry creation code.
       */
      void create(const GearDir& content, G4LogicalVolume& topVolume,
                  geometry::GeometryTypes type);

    private:

    };

  }

}

#endif

