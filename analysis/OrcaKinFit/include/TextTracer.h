/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      virtual void initialize(BaseFitter& fitter) override;
      /// Called at the end of each step
      virtual void step(BaseFitter& fitter) override;
      /// Called at intermediate points during a step
      virtual void substep(BaseFitter& fitter,
                           int flag
                          ) override;
      /// Called at the end of a fit
      virtual void finish(BaseFitter& fitter) override;

      void printFitObjects(BaseFitter& fitter);
      void printConstraints(BaseFitter& fitter);
      void printTraceValues(const BaseFitter& fitter);
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
