/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCSZFITTER_H
#define CDCSZFITTER_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the z coordinate over travel distance fit.
    class CDCSZFitter : public CDCLocalTracking::UsedTObject {
    public:

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */

      CDCSZFitter();

      /** Destructor. */
      ~CDCSZFitter();

      void update(CDCTrajectorySZ& trajectory, const CDCRecoHit3DVector& recohits) const {
        std::vector<FloatType> observations;
        fillObservations(recohits, observations);
        update(trajectory, observations);
      }

      void fillObservations(const CDCRecoHit3DVector& recohits, std::vector<FloatType>& observations) const {
        for (CDCRecoHit3DVector::const_iterator itRecoHit = recohits.begin();
             itRecoHit != recohits.end(); ++itRecoHit) {
          fillObservation(*itRecoHit, observations);
        }
      }

      void fillObservation(const Belle2::CDCLocalTracking::CDCRecoHit3D& recohit, std::vector<FloatType>& observations) const {
        observations.push_back(recohit.getPerpS());
        observations.push_back(recohit.getPos3D().z());
      }

    private:
      void update(CDCTrajectorySZ& trajectorySZ, std::vector<FloatType>& observations) const {

        size_t nObservations = observations.size() / 2;
        FloatType* rawObservations = &(observations.front());
        updateOptimizeZDistance(trajectorySZ, rawObservations, nObservations);

      }

      void updateOptimizeSZDistance(
        CDCTrajectorySZ& trajectorySZ,
        FloatType* observations,
        size_t nObservations
      ) const;

      void updateOptimizeZDistance(
        CDCTrajectorySZ& trajectorySZ,
        FloatType* observations,
        size_t nObservations
      ) const;

      /// ROOT Macro to make CDCSZFitter a ROOT class.
      ClassDefInCDCLocalTracking(CDCSZFitter, 1);

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCSZFITTER
