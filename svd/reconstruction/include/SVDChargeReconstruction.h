/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
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
     * Class for SVD Charge computation
     */
    class SVDChargeReconstruction : public SVDReconstructionBase {

    public:

      /**
       * all constructors implemented in SVDReconstructionBase are available
       */
      template<class ... T> explicit SVDChargeReconstruction(T ... args): SVDReconstructionBase(args ...) {};

      /**
       * virtual destructor
       */
      virtual ~SVDChargeReconstruction() {};

      /**
       * set that samples are in electrons already
       */
      void setSamplesAreInElectrons() {m_samplesAreInElectrons = true;};

      /** get strip charge as set in SVDRecoConfiguration payload if chargeAlgo = inRecoDBObject*/
      double getStripCharge(const TString& chargeAlgo = "fromRecoDBObject");
      /** get strip charge error as set in SVDRecoConfiguration payload if chargeAlgo = inRecoDBObject*/
      double getStripChargeError(const TString& chargeAlgo = "fromRecoDBObject");

      /** MaxSample Charge */
      double getMaxSampleCharge();
      /** MaxSample Charge Error*/
      double getMaxSampleChargeError();
      /** SumSamples Charge */
      double getSumSamplesCharge();
      /** SumSamples Charge Error*/
      double getSumSamplesChargeError();
      /** ELS3 Charge */
      double getELS3Charge();
      /** ELS3 Charge Error */
      double getELS3ChargeError();

    private:

      bool m_samplesAreInElectrons = false;/**<if true m_samples is in electrons*/
    };

  }

}

