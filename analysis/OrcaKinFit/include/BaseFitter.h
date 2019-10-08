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

#ifndef __BASEFITTER_H
#define __BASEFITTER_H

#include<vector>
#include<string>
#include<map>

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitObject;
    class BaseConstraint;
    class BaseHardConstraint;
    class BaseSoftConstraint;
    class BaseTracer;

//  Class BaseConstraint:
/// Abstract base class for fitting engines of kinematic fits
    /**
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2011/03/03 15:03:02 $
     *          by: $Author: blist $
     *
     */
    class BaseFitter {
    public:
      BaseFitter();
      virtual ~BaseFitter();
      virtual void addFitObject(BaseFitObject* fitobject_);
      virtual void addFitObject(BaseFitObject& fitobject_);
      virtual void addConstraint(BaseConstraint* constraint_);
      virtual void addConstraint(BaseConstraint& constraint_);
      virtual void addHardConstraint(BaseHardConstraint* constraint_);
      virtual void addHardConstraint(BaseHardConstraint& constraint_);
      virtual void addSoftConstraint(BaseSoftConstraint* constraint_);
      virtual void addSoftConstraint(BaseSoftConstraint& constraint_);
      virtual std::vector<BaseFitObject*>* getFitObjects();
      virtual std::vector<BaseHardConstraint*>* getConstraints();
      virtual std::vector<BaseSoftConstraint*>* getSoftConstraints();
      virtual double fit() = 0;
      virtual int getError() const = 0;
      virtual double getProbability() const = 0;
      virtual double getChi2() const = 0;
      virtual int    getDoF() const = 0;
      virtual int   getIterations() const = 0;

      virtual void reset();
      virtual bool initialize() = 0;

      virtual BaseTracer* getTracer();
      virtual const BaseTracer* getTracer() const;
      virtual void setTracer(BaseTracer* newTracer
                            );
      virtual void setTracer(BaseTracer& newTracer
                            );

      virtual const double* getGlobalCovarianceMatrix(int& idim  ///< 1st dimension of global covariance matrix
                                                     ) const;
      virtual double* getGlobalCovarianceMatrix(int& idim  ///< 1st dimension of global covariance matrix
                                               );

    protected:
      /// Copy constructor disabled
      BaseFitter(const BaseFitter& rhs);
      /// Assignment disabled
      BaseFitter& operator= (const BaseFitter& rhs);


      typedef std::vector <BaseFitObject*> FitObjectContainer;
      typedef std::vector <BaseHardConstraint*> ConstraintContainer;
      typedef std::vector <BaseSoftConstraint*> SoftConstraintContainer;

      typedef FitObjectContainer::iterator FitObjectIterator;
      typedef ConstraintContainer::iterator ConstraintIterator;
      typedef SoftConstraintContainer::iterator SoftConstraintIterator;

      FitObjectContainer      fitobjects;
      ConstraintContainer     constraints;
      SoftConstraintContainer softconstraints;

      int     covDim;   ///< dimension of global covariance matrix
      double* cov;      ///< global covariance matrix of last fit problem
      bool    covValid; ///< Flag whether global covariance is valid

#ifndef FIT_TRACEOFF
      BaseTracer* tracer;
#endif
    public:
#ifndef FIT_TRACEOFF
      std::map<std::string, double> traceValues;
#endif

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __BASEFITTER_H
