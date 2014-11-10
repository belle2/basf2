/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDBeamBackHitFilterModule_H
#define SVDBeamBackHitFilterModule_H

#include <framework/core/Module.h>
#include <simulation/dataobjects/BeamBackHit.h>

namespace Belle2 {
  namespace SVD {

    /** The SVDBeamBackHitFilter module.
     *
     * This module removes other than SVD-related BeamBackHits from the BeamBackHits StoreArray.
     */
    class SVDBeamBackHitFilterModule : public Module {

    public:
      /** Constructor defining the parameters */
      SVDBeamBackHitFilterModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the sorting */
      virtual void event();

    private:

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDBeamBackHitFilterModule_H
