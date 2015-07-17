/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {
    class FastHough {
    public:
      /**
       * @brief small helper function, to check if four values have the same sign
       */
      static inline bool sameSign(double n1, double n2, double n3, double n4)
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};
    };
  }
}
