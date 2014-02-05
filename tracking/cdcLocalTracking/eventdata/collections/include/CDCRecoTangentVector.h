/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOTANGENTVECTOR_H
#define CDCRECOTANGENTVECTOR_H

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoTangent.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCRecoTangent>
    /** See CDCGenHitVector for all methods and details */
    class CDCRecoTangentVector : public CDCGenHitVector<Belle2::CDCLocalTracking::CDCRecoTangent> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoTangentVector() {;}

      /// Empty deconstructor
      ~CDCRecoTangentVector() {;}

    private:
      /// ROOT Macro to make CDCRecoTangentVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoTangentVector, 1);

    }; //class CDCRecoTangentVector

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOTANGENTVECTOR_H
