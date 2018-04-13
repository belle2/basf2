/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
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

#include "analysis/OrcaKinFit/BaseTracer.h"

namespace Belle2 {
  namespace OrcaKinFit {

    BaseTracer::BaseTracer(): next(0) {}

    BaseTracer::~BaseTracer() {}

    void BaseTracer::initialize(BaseFitter& fitter)
    {
      if (next) next->initialize(fitter);
    }

    void BaseTracer::step(BaseFitter& fitter)
    {
      if (next) next->step(fitter);
    }

    void BaseTracer::substep(BaseFitter& fitter, int flag)
    {
      if (next) next->substep(fitter, flag);
    }

    void BaseTracer::finish(BaseFitter& fitter)
    {
      if (next) next->finish(fitter);
    }

    void BaseTracer::setNextTracer(BaseTracer* next_)
    {
      next = next_;
    }

    void BaseTracer::setNextTracer(BaseTracer& next_)
    {
      next = &next_;
    }

    BaseTracer* BaseTracer::getNextTracer()
    {
      return next;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace


