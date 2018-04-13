/*! \file
 *  \brief Declares class ParameterScanner
 *
 * \b Changelog:
 * -
 *
 * \b CVS Log messages:
 * - $Log: ParameterScanner.h,v $
 * - Revision 1.2  2011/03/16 16:33:24  mbeckman
 * - Compatibility fixes with ILCSoft svn
 * -
 * - Revision 1.1  2011/03/03 15:03:02  blist
 * - Latest version, with NewFitterGSL
 * -
 * - Revision 1.1  2010/05/25 15:02:11  blist
 * - ParameterScanner added
 * -
 * -
 *
 */
#ifdef MARLIN_USE_ROOT

#ifndef __PARAMETERSCANNER_H
#define __PARAMETERSCANNER_H

#include <vector>

namespace Belle2 {
  namespace OrcaKinFit {

    class BaseFitter;
    class BaseFitObject;
    class BaseHardConstraint;

    class ParameterScanner {
    public:
      ParameterScanner(BaseFitter& fitter_);

      void doScan(int xglobal,
                  int nx,
                  double xstart,
                  double xstop,
                  int yglobal,
                  int ny,
                  double ystart,
                  double ystop,
                  const char* idprefix = "",
                  const char* titleprefix = "",
                  double mumerit = 0);


    protected:

      typedef std::vector <BaseFitObject*> FitObjectContainer;
      typedef std::vector <BaseHardConstraint*> ConstraintContainer;

      typedef FitObjectContainer::iterator FitObjectIterator;
      typedef ConstraintContainer::iterator ConstraintIterator;

      BaseFitter& fitter;

      enum {NCONMAX = 100};


    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif /* #ifndef __PARAMETERSCANNER_H */

#endif // MARLIN_USE_ROOT
