/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTRANSFORMDATA_H
#define EKLMTRANSFORMDATA_H

/* External headers. */
#include <CLHEP/Geometry/Transform3D.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>

/**
 * @file
 * EKLM transformation and alignment data operations.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct TransformData
     * @brief Transformations.
     *
     * @var TransformData::endcap
     * Endcap transformations.
     *
     * @var TransformData::layer
     * Layer transformations.
     *
     * @var TransformData::sector
     * Sector transformations.
     *
     * @var TransformData::plane
     * Plane transformations.
     *
     * @var TransformData::strip
     * Strip transformations.
     *
     * @var TransformData::stripInverse
     * Inverse strip transformations.
     */
    struct TransformData {
      HepGeom::Transform3D endcap[2];
      HepGeom::Transform3D layer[2][14];
      HepGeom::Transform3D sector[2][14][4];
      HepGeom::Transform3D plane[2][14][4][2];
      HepGeom::Transform3D strip[2][14][4][2][75];
      HepGeom::Transform3D stripInverse[2][14][4][2][75];
    };

    /**
     * Fill transformations.
     * @param[out] dat Transformation data.
     */
    void fillTransforms(struct TransformData* dat);

    /**
     * Make transformations global from local.
     * @param[in,out] dat Transformation data.
     */
    void transformsToGlobal(struct TransformData* dat);

    /**
     * Get strip local to global transformation by hit.
     * @param[in] dat Transformation data.
     * @param[in] hit Hit.
     * @return Transformation.
     * @details
     * The transformation uses GEANT/CLHEP units (mm)!.
     */
    HepGeom::Transform3D* getStripLocalToGlobal(struct TransformData* dat,
                                                EKLMDigit* hit);

    /**
     * Get strip global to local transformation by hit.
     * @param[in] dat Transformation data.
     * @param[in] hit Hit.
     * @return Transformation.
     * @details
     * The transformation uses GEANT/CLHEP units (mm)!
     */
    HepGeom::Transform3D* getStripGlobalToLocal(struct TransformData* dat,
                                                EKLMDigit* hit);

  }

}

#endif

