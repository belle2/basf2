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

#include <tracking/dbobjects/DAFConfiguration.h>

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

    /** DAF Configuration Database OjbPtr */
    DBObjPtr<DAFConfiguration> m_DAFConfiguration;

    /** Track Fit type to select the proper DAFParameter from DAFConfiguration; by default c_Default */
    short m_trackFitType = DAFConfiguration::c_Default;

    /** Boolean to only show the warning about changed DAF parameters on first occurrence but not each time a DAF instance is created */
    mutable bool m_changedParametersMessageWasShown = false;
  };
}
