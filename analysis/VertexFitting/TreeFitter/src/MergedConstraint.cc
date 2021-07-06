/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//Merge multiple constraints that we want to project simultaneously.
//Kept for development, currently incompatible with referencing.

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/MergedConstraint.h>
#include <analysis/VertexFitting/TreeFitter/Projection.h>

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

