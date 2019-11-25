/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Sagar Hazra
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    double SVD_p(const Particle* part);
    double SVD_pTrue(const Particle* part);
    double SVD_dedx(const Particle* part);
    double SVD_nHits(const Particle* part);
    double SVD_CosTheta(const Particle* part);



  }
} // Belle2 namespace

