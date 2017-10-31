/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Initialise and calculate projection for IP constraint. Currently does nothing,
//doesn't pull beamspot data and no flag in the main module to request the constraint. (FIXME)

#include <analysis/modules/TreeFitter/InteractionPoint.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

namespace TreeFitter {
  extern int vtxverbose;


  InteractionPoint::InteractionPoint(Belle2::Particle* daughter, bool forceFitAll, int dimension) : ParticleBase("InteractionPoint"),
    m_constraintDimension(dimension), m_ipPos(3, 0), m_ipCov(3, 1), m_ipCovInv(3, 1)
  {
    //m_constraintDimension = dimension;
    addDaughter(daughter, forceFitAll);
    initBeamSpot();
  }

  ErrCode InteractionPoint::initBeamSpot()
  {
    ErrCode status;
    bool success = false;
    if (m_beamParams) {

      const TVector3& vertexVector = m_beamParams->getVertex();
      const TMatrixDSym& covVertex = m_beamParams->getCovVertex();
      /**
      std::cout << "x pos: " << vertexVector.x()  << std::endl;
      std::cout << "y pos: " << vertexVector.y()  << std::endl;
      std::cout << "z pos: " << vertexVector.z()  << std::endl;
      */
      if (m_constraintDimension == 3) {
        m_ipPos(1) = vertexVector.x();
        m_ipPos(2) = vertexVector.y();
        m_ipPos(3) = vertexVector.z();
        m_ipCov.fast(1, 1) = covVertex(0 , 0);
        m_ipCov.fast(2, 2) = covVertex(1 , 1);
        m_ipCov.fast(3, 3) = covVertex(2 , 2);
        m_ipCov.fast(2, 1) = covVertex(1 , 0);
        m_ipCov.fast(3, 1) = covVertex(2 , 0);
        m_ipCov.fast(3, 2) = covVertex(2 , 1);
      } else if (m_constraintDimension == 2) {
        m_ipPos(1) = vertexVector.x();
        m_ipPos(2) = vertexVector.y();
        m_ipCov.fast(1, 1) = covVertex(0 , 0);
        m_ipCov.fast(2, 2) = covVertex(1 , 1);
        m_ipCov.fast(2, 1) = covVertex(1 , 0);
      } else {
        B2ERROR("The IP constraint has to have a dimension of 2 or 3. This is currently not the case. ");
      }
      /**
      std::cout << "InteractionPoint: cov = (" << covVertex(0 , 0) << ", " << covVertex(0 , 1) << ", " << covVertex(0 , 2) << std::endl;
      std::cout << "InteractionPoint: cov = (" << covVertex(1 , 0) << ", " << covVertex(1 , 1) << ", " << covVertex(1 , 2) << std::endl;
      std::cout << "InteractionPoint: cov = (" << covVertex(2 , 0) << ", " << covVertex(2 , 1) << ", " << covVertex(2 , 2) << std::endl;
      */

      int ierr;
      m_ipCovInv = m_ipCov.inverse(ierr);
      if (!ierr) {
        success = true;
      }
    }

    if (!success) {
      B2WARNING("Couldnt init beam spot (vertex). Constraint will not be aplleid ");
      status = ErrCode::inversionerror;
    } else {
      B2DEBUG(80, "InteractionPoint: initial beam spot = ("
              << m_ipPos(1) << "," << m_ipPos(2) << "," << m_ipPos(3) << ")");
      B2DEBUG(80, "InteractionPoint: cov inverse diagonal = (" << m_ipCov(1 , 1) << ", " << m_ipCov(2 , 2) << ", " << m_ipCov(3 , 3));
    }
    return status;
  }


  ErrCode InteractionPoint::initPar1(FitParams* fitparams)
  {
    ErrCode status;
    int posindex = posIndex();
    int maxrow = m_constraintDimension;
    for (int row = 1; row <= maxrow; ++row) {
      fitparams->par()(posindex + row) = m_ipPos(row);
    }
    for (ParticleBase::conIter it = m_daughters.begin(); it != m_daughters.end() ; ++it) {
      status |= (*it)->initPar1(fitparams);
      status |= (*it)->initPar2(fitparams);
    }

    return status;
  }

  ErrCode InteractionPoint::initPar2(FitParams* fitparams __attribute__((unused)))
  {
    // nothing left to do: actually, should never be called
    assert(0);
    return ErrCode::success;
  }

  ErrCode InteractionPoint::initCov(FitParams* fitpar) const
  {
    ErrCode status;
    int posindex = posIndex();
    int maxrow = m_constraintDimension;
    for (int row = 1; row <= maxrow; ++row) {
      fitpar->cov().fast(posindex + row, posindex + row) = 1000 * m_ipCov.fast(row, row);
    }
    for (ParticleBase::conIter it = m_daughters.begin(); it != m_daughters.end(); ++it) {
      status |= (*it)->initCov(fitpar);
    }
    return status;
  }

  ErrCode InteractionPoint::projectIPConstraint(const FitParams& fitparams, Projection& p) const
  {
    int posindex = posIndex();
    int maxrow = m_constraintDimension;
    for (int row = 1; row <= maxrow; ++row) {
      p.r(row) = fitparams.par()(posindex + row) - m_ipPos(row);
      p.H(row, posindex + row) = 1;
      for (int col = 1; col <= row; ++col) {
        p.Vfast(row, col) = m_ipCov.fast(row, col);
      }
    }
    return ErrCode::success;
  }

  ErrCode InteractionPoint::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::beamspot:
        status |= projectIPConstraint(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

  double  InteractionPoint::chiSquare(const FitParams* fitparams) const
  {
    // calculate the chi2
    int posindex = posIndex();
    CLHEP::HepVector residual = m_ipPos - fitparams->par().sub(posindex + 1, posindex + 3);
    double chisq = m_ipCovInv.similarity(residual);
    // add the daughters
    chisq += ParticleBase::chiSquare(fitparams);

    return chisq;
  }

  void InteractionPoint::addToConstraintList(constraintlist& list, int depth) const
  {
    // first the daughters
    for (ParticleBase::conIter it = m_daughters.begin(); it != m_daughters.end(); ++it) {
      (*it)->addToConstraintList(list, depth - 1);
    }

    list.push_back(Constraint(this, Constraint::beamspot, depth, m_constraintDimension)) ;
  }
}
