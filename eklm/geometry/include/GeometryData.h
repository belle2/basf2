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

      /**
       * Read geometry data.
       */
      void read();

      /**
       * Get number of strips with different lengths.
       */
      int getNStripsDifferentLength();

      /**
       * Get index in length-based array.
       * @param[in] positionIndex index in position-based array.
       */
      int getStripLengthIndex(int positionIndex);

      /**
       * Get index in position-based array.
       * @param[in] positionIndex index in position-based array.
       */
      int getStripPositionIndex(int lengthIndex);

      /** Transformations. */
      TransformData m_TransformData;

    private:

      /** Number of strips in one plane. */
      int m_nStrip;

      /** Number of strips with different lengths in one plane. */
      int m_nStripDifferent;

      /** Number of strip in length-based array. */
      int* m_StripAllToLen;

      /** Number of strip in position-based array. */
      int* m_StripLenToAll;

    };

  }

}

#endif

