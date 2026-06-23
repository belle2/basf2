/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/fitter/GBLRecoFitterModule.h>
using namespace Belle2;

REG_MODULE(GBLRecoFitter)

/** Module for the GBL Fitter */
GBLRecoFitterModule::GBLRecoFitterModule() : BaseRecoFitterModule()
{
  setDescription("GBL Fitter using Genfit.");

  addParam("gblInternalIterations", m_param_gblInternalIterations, "M estimator iterations (c:Chaucy, h:Huber, t:Tukey)",
           m_param_gblInternalIterations);
  addParam("externalIterations", m_param_externalIterations,
           "Number of external iterations (with updated seed trajectory)", m_param_externalIterations);
  addParam("resolveAmbiguities", m_param_resolveAmbiguities,
           "Number of iteration up to which Ambiguities should be resolved", m_param_resolveAmbiguities);
  addParam("recalcJacobians", m_param_recalcJacobians,
           "Number of iteration up to which Jacobians should be recalculated / planes/meas updated after the fit. " \
           "0 = do not recalculate Jacobians. 1 = recalculate after first GBL fit. 2 = after 1st and 2nd GBL fit etc.",
           m_param_recalcJacobians);
  addParam("enableScatterers", m_param_enableScatterers,
           "If false, no scatterers will be added to GBL trajectory", m_param_enableScatterers);
  addParam("enableIntermediateScatterer", m_param_enableIntermediateScatterer,
           "True to simulate thick sctatterers by two thin scatterers 1st at detector plane and intermediate between each two planes",
           m_param_enableIntermediateScatterer);
}


/** Create a GBL fitter */
std::shared_ptr<genfit::AbsFitter> GBLRecoFitterModule::createFitter() const
{
  std::shared_ptr<genfit::GblFitter> fitter = std::make_shared<genfit::GblFitter>();
  fitter->setOptions(m_param_gblInternalIterations, m_param_enableScatterers, m_param_enableIntermediateScatterer,
                     m_param_externalIterations, m_param_recalcJacobians);

  return fitter;
}
