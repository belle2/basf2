/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/utilities/Ptr.h>
#include <tracking/trackFindingCDC/typedefs/InfoTypes.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HitPtr>
    class RLTagged : public StarToPtr<HitPtr> {

    private:
      /// Type of the base class
      using Super = StarToPtr<HitPtr>;

    public:
      /// Inheriting the constructor of the base class
      using Super::Super;

      /// Getter for the right left passage hypothesis
      RightLeftInfo getRLInfo() const
      { return m_rlInfo; }

      /// Setter for the right left passage hypothesis
      void setRLInfo(const RightLeftInfo& rlInfo)
      { m_rlInfo = rlInfo; }

    private:
      /// Memory for the right left passage hypotheses assoziated with the hit.
      RightLeftInfo m_rlInfo = UNKNOWN;

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
