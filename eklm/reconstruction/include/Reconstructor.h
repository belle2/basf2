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
#include <eklm/dataobjects/EKLMSectorHit.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/Digitizer.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Class for creation of 1d, 2d hits and cluster identification.
     */
    class Reconstructor {

    public:

      /**
       * Constructor.
       */
      Reconstructor(GeometryData* geoDat);

      /**
       * Destructor.
       */
      ~Reconstructor() {};

      /**
       * Read StripHits from the datastore.
       */
      void readStripHits();

      /**
       * Create Sector hits and fill it with StripHits.
       */
      void createSectorHits();

      /**
       * Create 2d hits in each SectorHit.
       */
      void create2dHits();

    private:

      /** Storage. */
      StoreArray<EKLMHit2d>m_hit2dArray;

      /** Vector of StripHits. */
      std::vector <EKLMDigit*> m_StripHitVector;

      /** Vector of SectorHits. */
      std::vector <EKLMSectorHit*> m_SectorHitVector;

      /**
       * Check whether hit has too small time.
       * @param[in] pos  Hit position.
       * @param[in] time HIt time.
       */
      bool fastHit(HepGeom::Point3D<double>& pos, double time);

      /** Digitization parameters. */
      struct DigitizationParams m_digPar;

      /** Geometry data. */
      GeometryData* m_geoDat;

    };

  }

}

#endif

