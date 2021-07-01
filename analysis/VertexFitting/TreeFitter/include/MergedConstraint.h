/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/Constraint.h>

namespace TreeFitter {
  /**
   * Merge multiple constraints that we want to project simultaneously.
   * As of release-00-08-00 this causes issues when referencing, see DecayChain.cc
   */
  class MergedConstraint : public Constraint {
  public:
    /** typedef of a list of constraints */
    typedef std::vector<Constraint*> constraintlist ;

    /** empty constructor */
    MergedConstraint() : Constraint(Constraint::merged) {}
    /** destructor */
    virtual ~MergedConstraint() {}
    /** constructor with constraint list */
    explicit MergedConstraint(const constraintlist& list) :
      Constraint(Constraint::merged), m_list(list)
    {
      int d = 0;
      for (constraintlist::iterator it = m_list.begin() ;
           it != m_list.end(); ++it) d += (*it)->dim() ;
      setDim(d) ;
    }



    /** project the constraints */
    virtual ErrCode project(const FitParams& fitpar, Projection& p) const override ;

    /** push back a new constraint */
    void push_back(Constraint* c)
    {
      m_list.push_back(c) ;
      setDim(dim() + c->dim()) ;
      setNIter(std::max(nIter(), c->nIter())) ;
    }


  private:
    /** list of the constraints to merge */
    constraintlist m_list ;
  } ;

}
