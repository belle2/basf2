/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * See https://github.com/tferber/OrcaKinfit, forked from                 *
 * https://github.com/iLCSoft/MarlinKinfit                                *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (ferber@physics.ubc.ca) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "analysis/OrcaKinFit/BaseFitter.h"
#include "analysis/OrcaKinFit/BaseSoftConstraint.h"
#include "analysis/OrcaKinFit/BaseHardConstraint.h"

#undef NDEBUG
#include <cassert>

namespace Belle2 {

  namespace OrcaKinFit {

    BaseFitter::BaseFitter()
      : fitobjects(FitObjectContainer()),
        constraints(ConstraintContainer()),
        softconstraints(SoftConstraintContainer()),
        covDim(0), cov(0), covValid(false)
#ifndef FIT_TRACEOFF
      , tracer(0),
        traceValues(std::map<std::string, double> ())
#endif
    {}

    BaseFitter::~BaseFitter()
    {
      delete[] cov;
      cov = 0;
    }

    void BaseFitter::addFitObject(BaseFitObject* fitobject_)
    {
      covValid = false;
      fitobjects.push_back(fitobject_);
    }

    void BaseFitter::addFitObject(BaseFitObject& fitobject_)
    {
      covValid = false;
      fitobjects.push_back(&fitobject_);
    }

    void BaseFitter::addConstraint(BaseConstraint* constraint_)
    {
      covValid = false;

      if (BaseHardConstraint* hc = dynamic_cast<BaseHardConstraint*>(constraint_))
        constraints.push_back(hc);
      else if (BaseSoftConstraint* sc = dynamic_cast<BaseSoftConstraint*>(constraint_))
        softconstraints.push_back(sc);
      else {
        // illegal constraint
        assert(0);
      }
    }

    void BaseFitter::addConstraint(BaseConstraint& constraint_)
    {
      covValid = false;
      if (BaseHardConstraint* hc = dynamic_cast<BaseHardConstraint*>(&constraint_))
        constraints.push_back(hc);
      else if (BaseSoftConstraint* sc = dynamic_cast<BaseSoftConstraint*>(&constraint_))
        softconstraints.push_back(sc);
    }

    void BaseFitter::addHardConstraint(BaseHardConstraint* constraint_)
    {
      covValid = false;
      constraints.push_back(constraint_);
    }

    void BaseFitter::addHardConstraint(BaseHardConstraint& constraint_)
    {
      covValid = false;
      constraints.push_back(&constraint_);
    }

    void BaseFitter::addSoftConstraint(BaseSoftConstraint* constraint_)
    {
      covValid = false;
      softconstraints.push_back(constraint_);
    }

    void BaseFitter::addSoftConstraint(BaseSoftConstraint& constraint_)
    {
      covValid = false;
      softconstraints.push_back(&constraint_);
    }

    std::vector<BaseFitObject*>* BaseFitter::getFitObjects()
    {
      return &fitobjects;
    }

    std::vector<BaseHardConstraint*>* BaseFitter::getConstraints()
    {
      return &constraints;
    }

    std::vector<BaseSoftConstraint*>* BaseFitter::getSoftConstraints()
    {
      return &softconstraints;
    }

    void BaseFitter::reset()
    {
      fitobjects.resize(0);
      constraints.resize(0);
      softconstraints.resize(0);
      covValid = false;
    }

    BaseTracer* BaseFitter::getTracer()
    {
      return tracer;
    }
    const BaseTracer* BaseFitter::getTracer() const
    {
      return tracer;
    }
    void BaseFitter::setTracer(BaseTracer* newTracer)
    {
      tracer = newTracer;
    }

    void BaseFitter::setTracer(BaseTracer& newTracer)
    {
      tracer = &newTracer;
    }

    const double* BaseFitter::getGlobalCovarianceMatrix(int& idim) const
    {
      if (covValid && cov) {
        idim = covDim;
        return cov;
      }
      idim = 0;
      return 0;
    }

    double* BaseFitter::getGlobalCovarianceMatrix(int& idim)
    {
      if (covValid && cov) {
        idim = covDim;
        return cov;
      }
      idim = 0;
      return 0;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

