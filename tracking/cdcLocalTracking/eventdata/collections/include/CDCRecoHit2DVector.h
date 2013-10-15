/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT2DVECTOR_H
#define CDCRECOHIT2DVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of reconstructed facets. Template instance of CDCGenericHitVector<CDCRecoHit2D>
    /** See CDCGenericHitVector for all methods and details. \n
      * Additionally this inherits from AutomataCell since we need it for the CDCRecoSegments.*/
    class CDCRecoHit2DVector : public AutomatonCell {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit2DVector() : AutomatonCell() {;}

      /// Empty deconstructor
      ~CDCRecoHit2DVector() {;}

      /* ###### poor mans collection template ###### */
    public:
      typedef CDCRecoHit2DVector Collection; ///< The type of this class
    private:
      typedef std::vector<Belle2::CDCLocalTracking::CDCRecoHit2D> Container; ///< std::vector to be wrapped

    public:
      typedef Container::value_type Item; ///< Value type of this container
      /*  typedef Container::value_type value_type; ///< Value type of this container

        typedef Container::iterator iterator; ///< Iterator type of this container
        typedef Container::const_iterator const_iterator; ///< Constant iterator type of this container

        typedef Container::reverse_iterator reverse_iterator; ///< Reversed iterator type of this container
        typedef Container::const_reverse_iterator const_reverse_iterator; ///< Constant reversed iterator type of this
      container*/

    public:
      /// Include all method implementations
#include <tracking/cdcLocalTracking/eventdata/collections/implementation/CDCGenericHitVector.part.h>
    private:
      /// ROOT Macro to make CDCRecoHit2DVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit2DVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCRECOHIT2DVECTOR_H
