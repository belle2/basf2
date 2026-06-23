/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <genfit/GblFitter.h>

namespace Belle2 {

  /** General Broken Lines (GBL) fitter. */
  class GBLRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the General Broken Lines (GBL) Fitter. */
    GBLRecoFitterModule();

  protected:
    /** Create a General Broken Lines (GBL) fitter. */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override;

  private:
    std::string m_param_gblInternalIterations = ""; /**< M estimator iterations*/
    unsigned int m_param_externalIterations = 1; /**< Number of external iterations */
    unsigned int m_param_resolveAmbiguities = 0 ; /**< Number of iteration up to which ambiguities should be resolved */
    unsigned int m_param_recalcJacobians = 1; /**< Number of iteration up to which Jacobians should be recalculated. */
    bool m_param_enableScatterers = true; /**< If false, no scatterers will be added to GBL trajectory */
    bool m_param_enableIntermediateScatterer = true; /**< True to simulate thick sctatterers by two thin scatterers */
  };
}
