/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <genfit/GblFitter.h>

namespace Belle2 {

  class GBLRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the GBL fitter. DO NOT USE THIS MODULE IN THE MOMENT! */
    GBLRecoFitterModule() : BaseRecoFitterModule()
    {
      setDescription("GBL Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                     "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                     "hits using the MeasurementCreatorModule.");

      addParam("internalIterations", m_gblInternalIterations,
               "GBL: internal down weighting setting (separated by ',' for each external iteration, e.g ',,Hh'", std::string(""));
      addParam("externalIterations", m_gblExternalIterations,
               "Number of times the GBL trajectory should be fitted and updated with results", int(1));
      addParam("enableScatterers", m_enableScatterers, "Enable scattering in GBL trajectory", bool(true));
      addParam("enableIntermediateScatterer", m_enableIntermediateScatterer,
               "Enable intermediate scatterers for simulation of thick scatterer", bool(true));
      addParam("recalcJacobians", m_recalcJacobians,
               "Recalculate Jacobians/planes: 0=do not recalculate, 1=after 1st fit, 2=1 & after 2nd fit, etc. Use '1' for 1 iteration + output to mille or if iteration>=2 ",
               int(0));

    }

  protected:
    /** Create a GBL fitter */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override
    {
      std::shared_ptr<genfit::GblFitter> fitter = std::make_shared<genfit::GblFitter>();

      fitter->setOptions(m_gblInternalIterations, m_enableScatterers, m_enableIntermediateScatterer, m_gblExternalIterations,
                         m_recalcJacobians);

      return fitter;
    }

  private:
    std::string m_gblInternalIterations;             /**< GBL internal down weighting setting */
    int m_gblExternalIterations;                     /**< Real GBL iteration with state updates */
    bool m_enableScatterers;                         /**< Enable scattering in GBL trajectory */
    bool m_enableIntermediateScatterer;              /**< Enable intermediate scatterers for simulation of thick scatterer */
    int m_recalcJacobians;                           /**< Recalculate Jacobians: 0=do not recalculate, 1=after 1st fit, 2=1+after 2nd fit, etc. */
  };
}
