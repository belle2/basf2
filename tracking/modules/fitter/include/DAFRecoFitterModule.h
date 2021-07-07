/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <genfit/DAF.h>

namespace Belle2 {

  /** DAF fitter. */
  class DAFRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the DAF fitter. */
    DAFRecoFitterModule();

  protected:
    /** Create a DAF fitter. */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override;

  private:
    /** Probability cut for the DAF. Any value between 0 and 1 possible. Common values are between 0.01 and 0.001. */
    double m_param_probabilityCut;

    /** If the delta PValue between two DAF iterations is small than this value, the iterative procedure will be terminated early.*/
    double m_param_deltaPValue;

    /** Maximum number of failed hits before aborting the fit. */
    int m_param_maxNumberOfFailedHits = -1;
  };
}
