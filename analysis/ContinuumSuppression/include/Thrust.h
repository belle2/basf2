/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT)           *
 *   Original module writen by M. Nakao for Belle                         *
 *   Ported to Belle II by P. Goldenzweig                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef THRUST_H
#define THRUST_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include <analysis/VariableManager/Variables.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  TVector3 calculateThrust(const std::vector<TVector3>& momenta)
  {

    decltype(momenta.begin()) p;
    decltype(momenta.begin()) q;
    decltype(momenta.begin()) loopcount;

    const auto begin = momenta.begin();
    const auto end = momenta.end();

    double sump = 0;
    for (p = begin; p != end; p++)
      sump += (*p).Mag();


    TVector3 rvec, Axis;

    // Thrust and thrust vectors

    double Thru = 0;
    for (p = begin; p != end; p++) {
      TVector3 rvec(*p);
      if (rvec.z() <= 0.0) rvec = -rvec;

      double s = rvec.Mag();
      if (s != 0.0) rvec *= (1 / s);

      for (loopcount = begin; loopcount != end; loopcount++) {
        TVector3 rprev(rvec);
        rvec = TVector3(); // clear

        for (q = begin; q != end; q++) {
          const TVector3 qvec(*q);
          rvec += (qvec.Dot(rprev) >= 0) ? qvec : - qvec;
        }

        for (q = begin; q != end; q++) {
          const TVector3 qvec(*q);
          if (qvec.Dot(rvec) * qvec.Dot(rprev) < 0) break;
        }

        if (q == end) break;
      }

      double ttmp = 0.0;
      for (q = begin; q != end; q++) {
        const TVector3 qvec = *q;
        ttmp += std::fabs(qvec.Dot(rvec));
      }
      ttmp /= (sump * rvec.Mag());
      rvec *= 1 / rvec.Mag();
      if (ttmp > Thru) {
        Thru = ttmp;
        Axis = rvec;
      }
    }
    Axis *= Thru;
    return Axis;
  }

} // Belle2 namespace

#endif


