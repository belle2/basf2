/*! \file
 *  \brief Declares class BaseTracer
 *
 * \b Changelog:
 * -
 *
 * \b CVS Log messages:
 * - $Log: BaseTracer.h,v $
 * - Revision 1.1  2009/09/01 09:48:12  blist
 * - Added tracer mechanism, added access to fit covariance matrix
 * -
 * -
 *
 */

#ifndef __BASETRACER_H
#define __BASETRACER_H

#include <vector>

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitter;
    class BaseFitObject;
    class BaseConstraint;
    class BaseHardConstraint;
    class BaseSoftConstraint;

//  Class BaseTracer
/// Abstract base class for trace objects of kinematic fits
    /**
     * A Tracer object is called during the fitting process
     * by the fit engine, to store information on how the fit
     * proceeded. This is useful for debugging purposes,
     * because
     * - it provides uniform debugging output across different fitters,
     * - it frees the fitter code from too much debug code,
     * - and it decouples the fit engine classes from other software
     *   such as Root, which may be used by the tracer.
     *
     * Author: Benno List
     * Last update: $Date: 2009/09/01 09:48:12 $
     *          by: $Author: blist $
     *
     */

    class BaseTracer {
    public:
      BaseTracer();

      virtual ~BaseTracer();

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

      virtual void setNextTracer(BaseTracer* next_);
      virtual void setNextTracer(BaseTracer& next_);
      virtual BaseTracer* getNextTracer();

    protected:

      typedef std::vector <BaseFitObject*> FitObjectContainer;
      typedef std::vector <BaseHardConstraint*> ConstraintContainer;
      typedef std::vector <BaseSoftConstraint*> SoftConstraintContainer;

      typedef FitObjectContainer::iterator FitObjectIterator;
      typedef ConstraintContainer::iterator ConstraintIterator;
      typedef SoftConstraintContainer::iterator SoftConstraintIterator;

      BaseTracer* next;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __BASETRACER_H
