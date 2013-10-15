/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT3DSET_H
#define CDCRECOHIT3DSET_H

#include <algorithm>
#include <set>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// An ordered set of reconstructed facets. Template instance of CDCGenericHitSet<CDCRecoHit3D>
    /** See CDCGenericHitSet for all methods and details */
    class CDCRecoHit3DSet : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit3DSet() {;}

      /// Empty deconstructor
      ~CDCRecoHit3DSet() {;}

      /* ###### poor mans set template ###### */
    public:
      typedef CDCRecoHit3DSet Collection; ///< The type of this class
    private:
      typedef std::set<Belle2::CDCLocalTracking::CDCRecoHit3D> Container; ///< std::set to be wrapped
    public:
      typedef Container::value_type Item; ///< Value type of this container

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitSet.part.h>


    public:
      /// Calculates the sum of all squared distances of the stored reconstructed hits in the z direction to the sz fit.
      /** Calculates the sum of all squared distances of the stored reconstructed hits \n
       *  in the z direction to the sz fit. This is only useful in case CDCRecoHit3D are stored, \n
       *  because they are the only entities transporting z and s information.*/
      FloatType getSquaredZDist(const CDCTrajectorySZ& trajectorySZ) const;

    private:
      /// ROOT Macro to make CDCRecoHit3DSet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit3DSet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHIT3DSET_H
