/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/dataobjects/TrackGen.h>

namespace Belle2 {
  /** The usual Track is defined with the ChargedStable ParticleType.
   *
   *  @sa TrackGen
   */
//typedef TrackGen<Const::ChargedStable> Track;
  typedef TrackGen Track;
}
