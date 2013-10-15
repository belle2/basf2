/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT3DVECTOR_H
#define CDCRECOHIT3DVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of reconstructed facets. Template instance of CDCGenericHitVector<CDCRecoHit3D>
    /** See CDCGenericHitVector for all methods and details */
    class CDCRecoHit3DVector : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit3DVector() {;}

      /// Empty deconstructor
      ~CDCRecoHit3DVector() {;}

      /* ###### poor mans collection template ###### */
    public:
      typedef CDCRecoHit3DVector Collection; ///< The type of this class
    private:
      typedef std::vector<Belle2::CDCLocalTracking::CDCRecoHit3D> Container; ///< std::vector to be wrapped

    public:
      typedef Container::value_type Item; ///< Value type of this container
      /* typedef Container::value_type value_type; ///< Value type of this container

       typedef Container::iterator iterator; ///< Iterator type of this container
       typedef Container::const_iterator const_iterator; ///< Constant iterator type of this container

       typedef Container::reverse_iterator reverse_iterator; ///< Reversed iterator type of this container
       typedef Container::const_reverse_iterator const_reverse_iterator; ///< Constant reversed iterator type of this container
       */
    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitVector.part.h>


    public:
      /// Calculates the sum of all squared distances of the stored reconstructed hits in the z direction to the sz fit.
      /** Calculates the sum of all squared distances of the stored reconstructed hits \n
       *  in the z direction to the sz fit. This is only useful in case CDCRecoHit3D are stored, \n
       *  because they are the only entities transporting z and s information.*/
      FloatType getSquaredZDist(const CDCTrajectorySZ& trajectorySZ) const;

    private:
      /// ROOT Macro to make CDCRecoHit3DVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit3DVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCRECOHIT3DVECTOR_H
