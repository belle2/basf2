/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#ifndef EKLMTRANSFORMDATA_H_
#define EKLMTRANSFORMDATA_H_

#include <CLHEP/Geometry/Transform3D.h>

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
     */
    struct TransformData {
      HepGeom::Transform3D endcap[2];
      HepGeom::Transform3D layer[2][14];
      HepGeom::Transform3D sector[2][14][4];
      HepGeom::Transform3D plane[2][14][4][2];
      HepGeom::Transform3D strip[2][14][4][2][75];
    };

    /**
     * Write transformations to file.
     * @param[in] file Name of file.
     * @param[in] dat  Transformation data.
     * @return 0  Successful.
     * @return -1 Error.
     */
    int writeTransforms(const char* file, struct TransformData* dat);

    /**
     * Read transformations from file.
     * @param[in]  file Name of file.
     * @param[out] dat  Transformation data.
     * @return 0  Successful.
     * @return -1 Error.
     */
    int readTransforms(const char* file, struct TransformData* dat);

    /**
     * Fill transformations.
     * @param[out] dat Transformation data.
     */
    void fillTransforms(struct TransformData* dat);

  }

}

#endif

