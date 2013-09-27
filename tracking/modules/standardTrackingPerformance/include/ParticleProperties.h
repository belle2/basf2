/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Ziegler                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEPROPERTIES_H_
#define PARTICLEPROPERTIES_H_

namespace Belle2 {
  struct ParticleProperties {
    // measured values
    double mass;
    double pt;
    double ptot;
    double cosTheta;
    // generated values
    double mass_gen;
    double pt_gen;
    double ptot_gen;
    double cosTheta_gen;
  };

}


#endif /* PARTICLEPROPERTIES_H_ */
