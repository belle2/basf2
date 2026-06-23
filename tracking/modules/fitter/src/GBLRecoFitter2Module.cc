/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/fitter/GBLRecoFitter2Module.h>
using namespace Belle2;

REG_MODULE(GBLRecoFitter2)

/** Module for the GBL2 Fitter */
GBLRecoFitter2Module::GBLRecoFitter2Module() : BaseRecoFitterModule()
{
  setDescription("GBL Fitter2 (with THICK scatterers and ambiguities) using GenFit.");

  addParam("gblInternalIterations", m_param_gblInternalIterations, "M estimator iterations (c:Chaucy, h:Huber, t:Tukey)",
           m_param_gblInternalIterations);
  addParam("externalIterations", m_param_externalIterations,
           "Number of external iterations (with updated seed trajectory)", m_param_externalIterations);
  addParam("resolveAmbiguities", m_param_resolveAmbiguities,
           "Number of iteration up to which Ambiguities should be resolved", m_param_resolveAmbiguities);
}


/** Create a GBL2 fitter */
std::shared_ptr<genfit::AbsFitter> GBLRecoFitter2Module::createFitter() const
{
  std::shared_ptr<genfit::GblFitter2> fitter = std::make_shared<genfit::GblFitter2>();
  fitter->setOptions(m_param_gblInternalIterations, m_param_externalIterations, m_param_resolveAmbiguities);

  return fitter;
}
