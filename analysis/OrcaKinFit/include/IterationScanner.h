/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/*! \file
 *  \brief Declares class IterationScanner
 *
 * \b Changelog:
 * - 20.11.10 BL: First version
 *
 * \b CVS Log messages:
 * - $Log: IterationScanner.h,v $
 * - Revision 1.1  2011/03/03 15:03:02  blist
 * - Latest version, with NewFitterGSL
 * -
 *
 */
#ifdef MARLIN_USE_ROOT

#ifndef __ITERATIONSCANNER_H
#define __ITERATIONSCANNER_H

#include <vector>

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitter;
    class BaseFitObject;
    class BaseHardConstraint;

    class IterationScanner {
    public:
      explicit IterationScanner(BaseFitter& fitter_);

      void doScan(int xglobal,
                  int nx,
                  double xstart,
                  double xstop,
                  int yglobal,
                  int ny,
                  double ystart,
                  double ystop,
                  const char* idprefix = "",
                  const char* titleprefix = "");


    protected:

      typedef std::vector <BaseFitObject*> FitObjectContainer;
      typedef std::vector <BaseHardConstraint*> ConstraintContainer;

      typedef FitObjectContainer::iterator FitObjectIterator;
      typedef ConstraintContainer::iterator ConstraintIterator;

      BaseFitter& fitter;


    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif /* #ifndef __ITERATIONSCANNER_H */

#endif // MARLIN_USE_ROOT
