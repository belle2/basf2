/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini,Jo-Frederik Krohn                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/Constraint.h>
#include <analysis/VertexFitting/TreeFitter/KalmanCalculator.h>

namespace TreeFitter {

  bool Constraint::operator<(const Constraint& rhs) const
  {
    // the simple way
    return m_type < rhs.m_type ||
           (m_type == rhs.m_type && m_depth < rhs.m_depth);

    // this is probably the second most complicated routine: how do we
    // order the constraints. there is one very special case:
    // Ks->pipi0 requires the pi0 mass constraints at the very
    // end. otherwise, it just doesn't work. in all other cases, we
    // prefer to fit 'down' the tree'. the 'external' constraints must
    // be filtered first, but soft pions must be fitted after the
    // geometric constraints of the D. You see, this is horrible.

    // if either of the two is external, or either of the two is a
    // mass constraint, we order by _type_

    if ((m_type <= Constraint::composite ||
         rhs.m_type <= Constraint::composite) ||
        (m_type >= Constraint::mass ||
         rhs.m_type >= Constraint::mass)) {
      return m_type < rhs.m_type ||
             (m_type == rhs.m_type && m_depth < rhs.m_depth);
    }
    // if not, we order by depth
    return m_depth < rhs.m_depth  ||
           (m_depth == rhs.m_depth && m_type < rhs.m_type);
  }

  ErrCode Constraint::project(const FitParams& fitpar, Projection& p) const
  {
    return m_node->projectConstraint(m_type, fitpar, p);
  }

  ErrCode Constraint::filter(FitParams* fitpar)
  {
    ErrCode status;
    Projection p(fitpar->getDimensionOfState(), m_dim);
    KalmanCalculator kalman(m_dim, fitpar->getDimensionOfState());

    //B2DEBUG(19, "Filtering: " << this->name() << " dim state " << fitpar->getDimensionOfState()
    //          << " dim contr " << m_dim);

    double chisq(0);
    int iter(0);
    bool finished(false) ;
    while (!finished && !status.failure()) {

      p.resetProjection();
      status |= project(*fitpar, p);

      if (!status.failure()) {

        status |= kalman.calculateGainMatrix(
                    p.getResiduals(),
                    p.getH(),
                    fitpar,
                    &p.getV(),
                    1 // weight
                  );

        if (!status.failure()) {

          kalman.updateState(fitpar);

          const double dchisqconverged = 0.001;
          double newchisq = kalman.getChiSquare();
          double dchisq = newchisq - chisq;
          bool diverging = iter > 0 && dchisq > 0;
          bool converged = fabs(dchisq) < dchisqconverged;
          finished  = ++iter >= m_maxNIter || diverging || converged;
          chisq = newchisq;
        }
      }
    }

    //B2DEBUG(19, "n---- Constraint::filter total iterations # " << iter << " chi2 /ndf " << chisq / m_dim <<  " final chi2 = " << chisq
    //          << " NDF" << m_dim << " for " << this->name());

    const unsigned int NDF = kalman.getConstraintDim();
    fitpar->addChiSquare(kalman.getChiSquare(), NDF);

    kalman.updateCovariance(fitpar);
    m_chi2 = kalman.getChiSquare();
    return status;
  }

  std::string Constraint::name() const
  {
    std::string rc = "unknown constraint!";
    switch (m_type) {
      case beamspot:     rc = "beamspot";   break;
      case beamenergy:   rc = "beamenergy"; break;
      case composite:    rc = "composite";  break;
      case resonance:    rc = "resonance";  break;
      case track:        rc = "track";      break;
      case photon:       rc = "photon";     break;
      case klong:        rc = "klong";     break;
      case kinematic:    rc = "kinematic";  break;
      case geometric:    rc = "geometric";  break;
      case mass:         rc = "mass";       break;
      case massEnergy:   rc = "massEnergy"; break;
      case lifetime:     rc = "lifetime";   break;
      case merged:       rc = "merged";     break;
      case conversion:   rc = "conversion"; break;
      case ntypes:
      case unknown:
        break;
    }
    return rc;
  }
}
