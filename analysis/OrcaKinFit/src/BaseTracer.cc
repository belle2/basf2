/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "analysis/OrcaKinFit/BaseTracer.h"

namespace Belle2 {
  namespace OrcaKinFit {

    BaseTracer::BaseTracer(): next(nullptr) {}

    BaseTracer::~BaseTracer() = default;

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


