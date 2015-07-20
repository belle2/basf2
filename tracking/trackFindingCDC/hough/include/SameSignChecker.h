/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
namespace Belle2 {
  namespace TrackFindingCDC {

    /** Small helper for checking the same sign of for or two floats.
     */
    class SameSignChecker {

    public:
      /// Check if four values have the same sign.
      static inline bool sameSign(float n1, float n2, float n3, float n4)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
      }

      /// Check if two values have the same sign.
      static inline bool sameSign(float n1, float n2)
      {
        return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
      }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
