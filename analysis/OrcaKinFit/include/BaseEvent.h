/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * See https://github.com/tferber/OrcaKinfit, forked from                 *
 * https://github.com/iLCSoft/MarlinKinfit                                *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (torben.ferber@desy.de) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __BASEEVENT_H
#define __BASEEVENT_H

#include "analysis/OrcaKinFit/FourVector.h"
#include "analysis/OrcaKinFit/BaseFitObject.h"
#include "analysis/OrcaKinFit/BaseFitter.h"

// class BaseEvent
/// Abstract base class for different kinds of events
/**
 * This class defines the minimal functionality of a class describing
 * an event hypothesis that is ment to be tested by a kinematic fit.
 * It is an optional class - the event hypothesis can also be defined
 * directly in the main program.
 *
 *
 * Author: Jenny Bme, Benno List
 * $Date: 2008/02/12 10:19:05 $
 * $Author: blist $
 *
 */

namespace Belle2 {
  namespace OrcaKinFit {

    class BaseEvent  {
    public:
      virtual ~BaseEvent() {};
      /// provides four-momenta (i.e. read values from ntuple, run toy MC, ...)
      virtual void genEvent() = 0;
      /// do it!
      virtual int fitEvent(BaseFitter& fitter) = 0;

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __BASEEVENT_H
