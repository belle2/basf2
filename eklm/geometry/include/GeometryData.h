/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMGEOMETRYDATA_H
#define EKLMGEOMETRYDATA_H

/* Belle2 headers. */
#include <eklm/geometry/TransformData.h>
#include <framework/utilities/FileSystem.h>

/**
 * @file
 * EKLM geometry data.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Geometry data class.
     */
    class GeometryData {

    public:

      /**
       * Constructor.
       * @param[in] global If true, load global transformations (false - local).
       */
      GeometryData(bool global);

      /**
       * Destructor.
       */
      ~GeometryData();

      /** Transformations. */
      TransformData m_TransformData;

    private:

    };

  }

}

#endif

