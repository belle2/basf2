/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCKARIMAKIFITTER_H
#define CDCKARIMAKIFITTER_H

#include "CDCFitter2D.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Karimaki fit for two dimensional trajectory circle
    class CDCKarimakiFitter : public CDCFitter2D<Belle2::CDCLocalTracking::KarimakisMethod> {

    public:
      ///Static getter for a general riemann fitter
      static const CDCKarimakiFitter& getFitter();

      ///Static getter for a line fitter
      //static const CDCKarimakiFitter& getLineFitter();

      ///Static getter for an origin circle fitter
      //static const CDCKarimakiFitter& getOriginCircleFitter();

    public:
      ///Empty constructor
      CDCKarimakiFitter();

      ///Empty destructor
      ~CDCKarimakiFitter();

    public:
      /** ROOT Macro to make CDCKarimakiFitter a ROOT class.*/
      CDCLOCALTRACKING_SwitchableClassDef(CDCKarimakiFitter, 1);

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCKARIMAKIFITTER
