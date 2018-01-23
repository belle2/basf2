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
//As of release-00-08-00 this causes issues when referencing, see DecayChain.cc

#include <framework/logging/Logger.h>
//
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/MergedConstraint.h>

namespace TreeFitter {

  ErrCode MergedConstraint::project(const FitParams& fitpar, Projection& p) const
  {
    B2DEBUG(80, "MergedConstraint:project()");
    ErrCode status ;
    for (constraintlist::const_iterator it = m_list.begin() ;
         it != m_list.end() ; ++it) {
      status |= (*it)->projectCopy(fitpar, p) ;
      p.incrementOffset((*it)->dim()) ;
    }
    return status ;
  }

}

