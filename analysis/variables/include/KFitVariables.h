/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {


    /**
     * return Chi2 of four momentum-constraint kinematical fit of KFit
     */
    double FourCKFitChi2(const Particle* part);

    /**
     * return Prob of four momentum-constraint kinematical fit of KFit
     */
    double FourCKFitProb(const Particle* part);


  }
} // Belle2 namespace
