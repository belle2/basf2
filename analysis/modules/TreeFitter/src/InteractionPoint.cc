/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Initialise and calculate projection for IP constraint. Currently does nothing,
//doesn't pull beamspot data and no flag in the main module to request the constraint. (FIXME)

#include <analysis/modules/TreeFitter/InteractionPoint.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/dataobjects/Particle.h>

namespace TreeFitter {
  extern int vtxverbose ;

  InteractionPoint::InteractionPoint(Particle* particle, bool forceFitAll)
    : InternalParticle(particle, 0, forceFitAll),
      m_constrainXY(false), m_constrainXYZ(false), m_ipPos(), m_ipCov(3),
      m_ipCovInv(3)//the last two matrices should be initialized to identity
  {
    //assert(particle->pdtEntry()->lundId()%1000 == PdtLund::Upsilon ) ;
    initBeamSpot(particle) ;
  }

  /*
  InteractionPoint::InteractionPoint(Particle* particle, bool forceFitAll, bool addupsilon)
    : InternalParticle(0, 0, forceFitAll),
      m_constrainXY(false), m_constrainXYZ(false), m_ipPos(3), m_ipCov(3),
      m_ipCovInv(3) //the last two matrices should be initialized to identity
  {
    addDaughter(particle, forceFitAll) ;
    initBeamSpot(particle) ;
  }
  */

  ErrCode InteractionPoint::initBeamSpot(Particle* particle __attribute__((unused)))
  {

    ErrCode status ;
    bool success = false ;
    // find the constraint
    /*
    const BtaConstraint* btaconstraint = particle->constraint(BtaConstraint::Beam) ; //adapt this
    if( btaconstraint ) {
      success =
    btaconstraint->getParmValue("bx",m_ipPos(1)) &&
    btaconstraint->getParmValue("by",m_ipPos(2)) &&
    btaconstraint->getParmValue("bz",m_ipPos(3)) &&
    btaconstraint->getParmValue("sxx",m_ipCov.fast(1,1)) &&
    btaconstraint->getParmValue("syy",m_ipCov.fast(2,2)) &&
    btaconstraint->getParmValue("szz",m_ipCov.fast(3,3)) &&
    btaconstraint->getParmValue("sxy",m_ipCov.fast(2,1)) &&
    btaconstraint->getParmValue("sxz",m_ipCov.fast(3,1)) &&
    btaconstraint->getParmValue("syz",m_ipCov.fast(3,2)) ;
      m_constrainXY = m_constrainXYZ = success ;
      if(success) {
    int ierr ;
    m_ipCovInv = m_ipCov.inverse(ierr) ;
      }
    }
    */
    if (!success) {
      std::cout << "Error"  << "failed to get beam spot data. constraint will not be applied." << std::endl;
    } else {
      if (vtxverbose >= 2)
        std::cout << "InteractionPoint: initial beam spot = ("
                  << m_ipPos(1) << "," << m_ipPos(2) << "," << m_ipPos(3) << ")" << std::endl ;
    }
    return status ;
  }

  InteractionPoint::~InteractionPoint() {}

  ErrCode InteractionPoint::initPar1(FitParams* fitparams)
  {
    ErrCode status ;
    int posindex = posIndex() ;
    for (int row = 1; row <= 3; ++row)
      fitparams->par()(posindex + row) = m_ipPos(row);

    for (daucontainer::const_iterator it = daughters().begin() ;
         it != daughters().end() ; ++it) {
      status |= (*it)->initPar1(fitparams) ;
      status |= (*it)->initPar2(fitparams) ;
    }

    return status ;
  }

  ErrCode InteractionPoint::initPar2(FitParams* fitparams __attribute__((unused)))
  {
    // nothing left to do: actually, should never be called
    assert(0) ;
    return ErrCode::success ;
  }

  ErrCode InteractionPoint::initCov(FitParams* fitpar) const
  {
    ErrCode status ;
    int posindex = posIndex() ;
    for (int row = 1; row <= 3; ++row)
      fitpar->cov().fast(posindex + row, posindex + row)
        = 1000 * m_ipCov.fast(row, row);

    for (daucontainer::const_iterator it = daughters().begin() ;
         it != daughters().end() ; ++it)
      status |= (*it)->initCov(fitpar) ;

    return status ;
  }

  ErrCode InteractionPoint::projectIPConstraint(const FitParams& fitparams, Projection& p) const
  {
    int posindex = posIndex() ;
    int maxrow = m_constrainXYZ ? 3 : (m_constrainXY ? 2 : 0) ;
    for (int row = 1; row <= maxrow; ++row) {
      p.r(row) = fitparams.par()(posindex + row) - m_ipPos(row) ;
      p.H(row, posindex + row) = 1 ;
      for (int col = 1; col <= row; ++col)
        p.Vfast(row, col) = m_ipCov.fast(row, col);
    }
    return ErrCode::success ;
  }

  ErrCode InteractionPoint::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status ;
    switch (type) {
      case Constraint::beamspot:
        status |= projectIPConstraint(fitparams, p) ;
        break ;
      default:
        status |= InternalParticle::projectConstraint(type, fitparams, p) ;
    }
    return status ;
  }

  double  InteractionPoint::chiSquare(const FitParams* fitparams) const
  {
    // calculate the chi2
    int posindex = posIndex() ;
    HepVector residual = m_ipPos - fitparams->par().sub(posindex + 1, posindex + 3) ;
    double chisq = m_ipCovInv.similarity(residual);

    // add the daughters
    chisq += InternalParticle::chiSquare(fitparams) ;

    return chisq ;
  }

  void InteractionPoint::addToConstraintList(constraintlist& alist, int depth) const
  {
    // first the beamspot
    int dim = m_constrainXYZ ? 3 : (m_constrainXY ? 2 : 0) ;
    alist.push_back(Constraint(this, Constraint::beamspot, depth, dim)) ;

    // then the base class
    InternalParticle::addToConstraintList(alist, depth) ;
  }

}
