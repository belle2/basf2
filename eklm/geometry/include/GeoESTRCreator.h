/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Takanori Hara, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOESTRCREATOR_H
#define GEOESTRCREATOR_H

/* Belle2 headers. */
#include <framework/gearbox/GearDir.h>
#include <geometry/CreatorBase.h>

/**
 * @file
 * ESTR geometry.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct ESTRGeometryParams
     * @brief ESTR geometry parameters.
     *
     * @var ESTRGeometryParams::phi
     * The starting angle of the octagonal Endcap KLM shape.
     *
     * @var ESTRGeometryParams::dphi
     * The opening angle (shape is extended from phi to phi+dphi).
     *
     * @var ESTRGeometryParams::nsides
     * The number of sides (=8 : octagonal).
     *
     * @var ESTRGeometryParams::nboundary
     * The number of boundaries perpendicular to the z-axis.
     *
     * @var ESTRGeometryParams::z
     * The z coordinate of the section specified by input id (=boundary id).
     *
     * @var ESTRGeometryParams::rmin
     * The radius of the circle tangent to the sides of the inner polygon.
     *
     * @var ESTRGeometryParams::rmax
     * The radius of the circle tangent to the sides of the outer polygon.
     *
     * @var ESTRGeometryParams::zsub
     * The length of the tube.
     *
     * @var ESTRGeometryParams::rminsub
     * The inner radius of the tube.
     *
     * @var ESTRGeometryParams::rmaxsub
     * The outer radius of the tube.
     */
    struct ESTRGeometryParams {
      double phi;
      double dphi;
      int nsides;
      int nboundary;
      double* z;
      double* rmin;
      double* rmax;
      double zsub;
      double rminsub;
      double rmaxsub;
    };

    /**
     * Read ESTR geometry parameters.
     * @param par Geometry parameters.
     * @return 0      Successful.
     * @return ENOMEM Memory allocation error.
     * @details
     * Allocates memory for z, rmin and rmax arrays.
     */
    int readESTRData(struct ESTRGeometryParams* par);

  }

}

#endif

