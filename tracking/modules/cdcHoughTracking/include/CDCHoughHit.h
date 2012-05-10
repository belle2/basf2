/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCHOUGHHIT_H
#define CDCHOUGHHIT_H

#include "TVector3.h"
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/modules/cdcHoughTracking/Angle.h>


namespace Belle2 {
  namespace Tracking {


    /** CDC Hit Class used for pattern recognition. */
    class CDCHoughHit {
    public:
      CDCHoughHit();
      CDCHoughHit(const CDCHit& hit, const unsigned int index);
      ~CDCHoughHit();

      Angle getPhi() const {
        return m_phi;
      }

      float getR() const {
        return m_r;
      }

      float getX() const {
        return m_x;
      }

      float getY() const {
        return m_y;
      }

      const TVector3& getBackwardPosition() const {
        return m_backwardPosition;
      }

      const TVector3& getForwardPosition() const {
        return m_forwardPosition;
      }

      unsigned int getStoreIndex() const {
        return m_storeIndex;
      }

      unsigned int getSuperlayerId() const {
        return m_superlayerId;
      }

      bool getIsAxial() const {
        return m_isAxial;
      }

    private:
      unsigned int m_storeIndex;
      unsigned int m_superlayerId;
      bool m_isAxial;
      float m_x;
      float m_y;
      float m_r;
      Angle m_phi;
      TVector3 m_forwardPosition;
      TVector3 m_backwardPosition;

    }; //end class

    //helper struct to sort hits by radius using std::sort
    struct SortHoughHitsByRadius: public std::binary_function < CDCHoughHit,
        CDCHoughHit, bool > {
      bool operator()(const CDCHoughHit& lhs, const CDCHoughHit& rhs);
    };

  }
} //end namespaces

#endif
