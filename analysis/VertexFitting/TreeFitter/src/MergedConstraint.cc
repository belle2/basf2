/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Merge multiple constraints that we want to project simultaneously.
//Kept for development, currently incompatible with referencing.

#include <framework/logging/Logger.h>
//
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/MergedConstraint.h>

namespace TreeFitter {

  ErrCode MergedConstraint::project(const FitParams& fitpar, Projection& p) const
  {
    ErrCode status ;
    for (auto element : m_list) {
      status |= element->project(fitpar, p) ;
      p.incrementOffset(element->dim()) ;
    }
    return status ;
  }
}

