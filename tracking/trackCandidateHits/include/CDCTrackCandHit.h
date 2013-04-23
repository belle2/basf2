/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Moritz Nadler                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCTrackCandHit_H
#define CDCTrackCandHit_H

#include <TObject.h>
#include <GFTrackCandHit.h>

namespace Belle2 {
  /** Namespace for everything inside the tracking package that is not a module or a datastore class*/
  namespace Tracking {
    /** This class extends the capabilities of the GFTrackCandHit to store how the left right ambiguity of CDC hits should be resolved.
     * Track finder that can determine this information should use this class and give it to the GFTrackCand. The information will then be extracted
     * by the GenFitter module and given to Genfit*/
    class CDCTrackCandHit : public GFTrackCandHit {
    public:
      /** Constructor. First 4 arguments are identical GFTrackCandHit's constructor. 5th argument tells how to resolve the ambiguity*/
      CDCTrackCandHit(int detId = -1, int hitId = -1, int planeId = -1, double rho =  0., char leftOrRight = 0):
        GFTrackCandHit(detId, hitId, planeId, rho), m_leftOrRight(leftOrRight) {

      }
      /** get the info how the left right ambiguity of CDC hits should be resolved*/
      char getLeftRightResolution() const {return m_leftOrRight;}
      /** set the info how the left right ambiguity of CDC hits should be resolved*/
      void setLeftRightResolution(char leftOrRight) {
        m_leftOrRight = leftOrRight;
      }
      /** clone function is needed so this object gets copied correctly inside Genfit*/
      CDCTrackCandHit* clone() const {return new CDCTrackCandHit(*this);}
    protected:

      char m_leftOrRight; /**< to store how the left right ambiguity of CDC hits should be resolved*/

//public:
//  ClassDef(CDCTrackCandHit, 1);

    };

  }
}

#endif
