/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/SVDReconstructionBase.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD strip charge
     */
    class SVDChargeReconstruction : public SVDReconstructionBase {

    public:

      /**
       * virtual destructor
       */
      virtual ~SVDChargeReconstruction() {};


      /** get strip charge as set in SVDRecoConfiguration payload*/
      double getStripCharge();
      /** get strip charge error as set in SVDRecoConfiguration payload*/
      double getStripChargeError();

      /** CoG6 Charge */
      double getMaxSampleCharge();
      /** CoG6 Charge Error*/
      double getMaxSampleChargeError();
      /** CoG3 Charge */
      double getSumSamplesCharge();
      /** CoG3 Charge Error*/
      double getSumSamplesChargeError();
      /** ELS3 Charge */
      double getELS3StripCharge() {return 0;}
      /** ELS3 Charge Error */
      double getELS3StripChargeError() {return 0;}

    protected:

    };

  }

}

