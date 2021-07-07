/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>

namespace TreeFitter {
  class ParticleBase ;
  class Projection ;
  class FitParams ;

  /** class to manage the order of constraints and their filtering */
  class Constraint {
  public:
    /**
     *  type of constraints
     *  the order of these constraints is important: it is the order in
     *  which they are applied.
    */
    enum Type { unknown = 0,
                beamenergy,
                beamspot,
                origin,
                lifetime,
                resonance,
                composite,
                track,
                photon,
                klong,
                conversion,
                kinematic,
                geometric,
                mass,
                massEnergy,
                merged,
                ntypes,
                helix
              };

    /** operator used to sort the constraints */
    bool operator<(const Constraint& rhs) const;

    /** operator */
    bool operator==(const Constraint& rhs) const
    {
      return m_type == rhs.m_type;
    }

    /**  get type of constraint */
    Type type() const { return m_type; }

    /**get dimension of constraint */
    unsigned int dim() const { return m_dim; }

    /** is this a linear constraint */
    bool isLinear() const { return m_maxNIter <= 1; }

    /**  get maximum number of iterations for non in constraint */
    unsigned int nIter() const { return m_maxNIter; }

    /** constructor  */
    Constraint() :
      m_node(0),
      m_depth(0),
      m_type(unknown),
      m_dim(0),
      m_weight(0),
      m_maxNIter(0) {}

    /** constructor */
    Constraint(const ParticleBase* node,
               Type type,
               int depth,
               unsigned int dim,
               int maxniter = 1) :
      m_node(node),
      m_depth(depth),
      m_type(type),
      m_dim(dim),
      m_weight(1),
      m_maxNIter(maxniter) {}

    /** destructor */
    virtual ~Constraint() {}

    /**   call the constraints projection function */
    virtual ErrCode project(const FitParams& fitpar, Projection& p) const;

    /** filter this constraint */
    virtual ErrCode filter(FitParams& fitpar);

    /** filter this constraint */
    virtual ErrCode filterWithReference(FitParams& fitpar, const FitParams& oldState);

    /** get name of constraint  */
    std::string name() const;

    /**
     * used to be able to weight the constraints
     * */
    [[gnu::unused]] void setWeight(int w) { m_weight = w < 0 ? -1 : 1; }

  protected:

    /**  constructor */
    explicit Constraint(Constraint::Type type) :
      m_node(0),
      m_depth(0),
      m_type(type),
      m_dim(0),
      m_weight(0),
      m_maxNIter(0) {}

    /**   set dimension of constraint */
    void setDim(unsigned int d) { m_dim = d; }

    /** set max number of iterations for non lin constraint  */
    void setNIter(unsigned int d) { m_maxNIter = d; }

  private:

    /** particle behind the constraint  */
    const ParticleBase* m_node;

    /** chi2 coming from the constraint */

    /** depth of the constraint in the tree
     * (determines for example the order of the track constraints)  */
    int m_depth;

    /**  type of constraint */
    Type m_type;

    /**  dimension of constraint */
    unsigned int m_dim;

    /** weight of this constraint currently we set them all to unity  */
    int m_weight;

    /** maximum number of iterations for non-linear constraints    */
    int m_maxNIter;
  };

}
