/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECON_H
#define EKLMRECON_H

/* C++ headers. */
#include <vector>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMSectorHit.h>
#include <eklm/geoeklm/TransformData.h>
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
      Reconstructor(struct TransformData* transf);

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

      /** Vector of 2d hits. */
      std::vector <EKLMHit2d*> m_hit2dVector;

      /**
       * Check strip orientation.
       * @param[in] hit Hit.
       * @return true if strip is along X.
       */
      bool CheckStripOrientationX(EKLMDigit* hit);

      /**
       * Check whether strips intersect.
       * @details
       * Determines crossing point in the global rest frame,
       * calculates Chi^2 of the hit and hittime.
       * @param[in]  hit1       First hit.
       * @param[in]  hit2       Second hit.
       * @param[out] crossPoint Crossing point.
       * @param[out] chisq      Chi^2.
       * @param[out] time       Time.
       * @return true if strips have intersection.
       */
      bool doesIntersect(EKLMDigit* hit1, EKLMDigit* hit2,
                         TVector3& crossPoint, double& chisq, double& time);

      /**
       * Add strip hit to sector.
       * @param[in,out] sectorHit Sector hit.
       * @param[in]     stripHit  Strip hit.
       * @return true on success.
       */
      bool addStripHitToSector(EKLMSectorHit* sectorHit, EKLMDigit* stripHit);

      /** Speed of the first photon. */
      double m_firstPhotonlightSpeed;

      /** Time smearing. */
      double m_sigmaT;

      /** Transformation data. */
      struct TransformData* m_transf;

    };

  }

}

#endif

