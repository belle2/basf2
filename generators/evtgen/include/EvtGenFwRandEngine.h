/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

