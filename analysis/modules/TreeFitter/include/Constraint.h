/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <string>
#include <iostream>
#include <analysis/modules/TreeFitter/ErrCode.h>

namespace TreeFitter {
  class ParticleBase ;
  class Projection ;
  class FitParams ;

  class Constraint {
  public:
    /**
     *  type of constraints
     *  the order of these constraints is important: it is the order in
     *  which they are applied.
    */
    enum Type { unknown = 0,
                beamspot,
                beamenergy,
                lifetime,
                resonance,
                composite,
                track,
                photon,
                conversion,
                kinematic,
                massEnergy,
                geometric,
                mass,
                merged,
                ntypes
              };

    /** operator */
    bool operator<(const Constraint& rhs) const;

    /** operator */
    bool operator==(const Constraint& rhs) const
    {
      return m_type == rhs.m_type;
    }

    // accessors
    /**  get type of constraint */
    Type type() const { return m_type; }

    /**get dimension of constraint */
    unsigned int dim() const { return m_dim; }

    /** s the constraint linear */
    bool isLinear() const { return m_maxNIter <= 1; }

    /**  get maximum number of iterations for non in contraint */
    unsigned int nIter() const { return m_maxNIter; }

    /** constructor  */
    Constraint() : m_node(0), m_depth(0), m_type(unknown), //FT: Now fully initialised, but rarely used
      m_dim(0), m_nHidden(0), m_weight(0), m_maxNIter(0) {}

    /** constructor */
    Constraint(const ParticleBase* node, Type type, int depth,
               unsigned int dim, unsigned int nhidden = 0,
               int maxniter = 1) //, double precision=1e-5)
      : m_node(node), m_depth(depth), m_type(type), m_dim(dim),
        m_nHidden(nhidden), m_weight(1), m_maxNIter(maxniter) {}

    /** destructor */
    virtual ~Constraint() {}

    /**   call the constraints projection function FIXME its weird that this is buried in particle */
    virtual ErrCode project(const FitParams& fitpar, Projection& p) const;

    /** filter around x=0 */
    virtual ErrCode filter(FitParams* fitpar) ;

    //FT: new version of filtering
    /** does the filtering */
    virtual ErrCode filter(FitParams* fitpar, const FitParams* reference) ;

    /** print FIXME becomes obsolete woth verbose removal   */
    virtual void print(std::ostream& os = std::cout) const;

    /** get name of constraint  */
    std::string name() const;

    /** set to minus one if constraints needs to be removed on next filter
     * JFK dont understand, unused keep for comment above
     * */
    void setWeight(int w) { m_weight = w < 0 ? -1 : 1; }

    /** get chi2 over ndf for last kalmaniteration */
    double getChi2() const {return m_chi2;}

    /** */
    double getNDF()const {return m_ndf;}

  protected:

    /**  constructor */
    Constraint(Constraint::Type type) :
      m_node(0), m_depth(0), m_type(type), m_dim(0), m_nHidden(0),
      m_weight(0), m_maxNIter(0) {}

    /**   set diension of cosntraint */
    void setDim(unsigned int d) { m_dim = d; }

    /** set number of iterations for non lin constraint  */
    void setNIter(unsigned int d) { m_maxNIter = d; }

  private:

    /** particle behind the constraint  */
    const ParticleBase* m_node;

    /**chi2  of this constraint not divided by ndf! */
    double m_chi2;

    /** ndf */
    double m_ndf;

    /**  */
    int m_depth;

    /**  type of constraint */
    Type m_type;

    /**  dimension of constraint */
    unsigned int m_dim;

    /** the number of hidden 'degrees of freedom'. always zero except for the 'photon' constraint  */
    unsigned int m_nHidden;

    /**  dont understand, unused FIXME  */
    int m_weight;

    /** maximum number of iterations for non-linear constraints    */
    int m_maxNIter;
  };

}

#endif //CONSTRAINT_H
