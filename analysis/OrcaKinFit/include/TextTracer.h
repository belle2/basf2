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

#ifndef __TEXTTRACER_H
#define __TEXTTRACER_H

#include <iostream>
#include "analysis/OrcaKinFit/BaseTracer.h"

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitter;

//  Class TextTracer:
/// Class to produce text output during kinematic fits
    /**
     *
     * Author: Benno List
     * Last update: $Date: 2011/05/03 13:18:48 $
     *          by: $Author: blist $
     *
     */

    class BaseFitter;

    class TextTracer: public BaseTracer {
    public:
      explicit TextTracer(std::ostream& os_);
      virtual ~TextTracer();

      /// Called at the start of a new fit (during initialization)
      virtual void initialize(BaseFitter& fitter);
      /// Called at the end of each step
      virtual void step(BaseFitter& fitter);
      /// Called at intermediate points during a step
      virtual void substep(BaseFitter& fitter,
                           int flag
                          );
      /// Called at the end of a fit
      virtual void finish(BaseFitter& fitter);

      void printFitObjects(BaseFitter& fitter);
      void printConstraints(BaseFitter& fitter);
      void printTraceValues(BaseFitter& fitter);
      void printSums(BaseFitter& fitter);

    protected:
      std::ostream& os;

      int istep;
      int isubstep;
      double chi2fo;
      double chi2sc;
      double sumhc;
      double sumhcscal;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __TEXTTRACER_H
