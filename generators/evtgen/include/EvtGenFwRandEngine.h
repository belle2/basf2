/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz, Andreas Moll             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTGENFWRANDENGINE_H
#define EVTGENFWRANDENGINE_H

#include <evtgen/EvtGenBase/EvtRandomEngine.hh>
#include <TRandom3.h>


class EvtGenFwRandEngine : public EvtRandomEngine {
public:

  /**
   * Constructor.
   */

  virtual double random() {
    return gRandom->Uniform(1.0);
  }
};

#endif /* EVTGENFWRANDENGINE_H */
