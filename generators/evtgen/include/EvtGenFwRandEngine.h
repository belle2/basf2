/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz, Andreas Moll             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <EvtGenBase/EvtRandomEngine.hh>
#include <TRandom3.h>

//! Evtgen random generator
class EvtGenFwRandEngine : public EvtRandomEngine {
public:

  /* Constructor. */
  virtual double random()
  {
    return gRandom->Uniform(1.0);  /**< Function random */
  } //! end of random()
}; //! end of EvtRandomEngine

