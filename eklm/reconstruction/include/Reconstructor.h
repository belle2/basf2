/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECONSTRUCTOR_H
#define EKLMRECONSTRUCTOR_H

/* C++ headers. */
#include <vector>

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/geometry/TransformData.h>
#include <eklm/simulation/Digitizer.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Class for creation of 1d, 2d hits and cluster identification.
     */
    class Reconstructor {

      /**
       * Time calibration: parameters of time correction function
       */
      struct TimeParams {
        float p0; /**< Coefficient of the polynomial. */
        float p1; /**< Coefficient of the polynomial. */
      };

    public:

      /**
       * Constructor.
       */
      Reconstructor(TransformData* transformData);

      /**
       * Destructor.
       */
      ~Reconstructor();

      /**
       * Create 2d hits.
       */
      void create2dHits();

    private:

      /**
       * Get 2d hit time corresponding to EKLMDigit.
       * @param[in] d    EKLMDigit.
       * @param[in] dist Distance from 2d hit to SiPM.
       */
      double getTime(EKLMDigit* d, double dist);

      /**
       * Check whether hit has too small time.
       * @param[in] pos  Hit position.
       * @param[in] time HIt time.
       */
      bool fastHit(HepGeom::Point3D<double>& pos, double time);

      /** Digitization parameters. */
      struct DigitizationParams m_digPar;

      /** Transformation data. */
      TransformData* m_TransformData;

      /** Geometry data. */
      const EKLM::GeometryData* m_GeoDat;

      /** Time calibration data. */
      struct TimeParams* m_TimeParams;

    };

  }

}

#endif

