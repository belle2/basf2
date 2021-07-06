/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDBeamBackHitFilterModule_H
#define SVDBeamBackHitFilterModule_H

#include <framework/core/Module.h>

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
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDBeamBackHitFilterModule_H
