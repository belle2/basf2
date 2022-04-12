/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/Constraint.h>
#include <analysis/VertexFitting/TreeFitter/KalmanCalculator.h>

namespace TreeFitter {

  bool Constraint::operator<(const Constraint& rhs) const
  {
    return m_depth < rhs.m_depth  ||
           (m_depth == rhs.m_depth && m_type < rhs.m_type);
  }

  ErrCode Constraint::project(const FitParams& fitpar, Projection& p) const
  {
    return m_node->projectConstraint(m_type, fitpar, p);
  }

  ErrCode Constraint::filter(FitParams& fitpar)
  {
    /**
     * We don't have reference state yet so we use the k-1 last state
     * to linearize non-linear constraints
     * */
    ErrCode status;
    Projection p(fitpar.getDimensionOfState(), m_dim);
    KalmanCalculator kalman(m_dim, fitpar.getDimensionOfState());

    double chisq(0);
    int iter(0);
    bool finished(false) ;

    double accumulated_chi2 = 0;
    while (!finished && !status.failure()) {

      p.resetProjection();
      status |= project(fitpar, p);

      if (!status.failure()) {

        status |= kalman.calculateGainMatrix(
                    p.getResiduals(),
                    p.getH(),
                    fitpar,
                    &p.getV(),
                    1
                  );

        if (!status.failure()) {
          kalman.updateState(fitpar);

          // r R^-1 r
          double newchisq = kalman.getChiSquare();

          double dchisqconverged = 0.001 ;

          double dchisq = newchisq - chisq;
          bool diverging = iter > 0 && dchisq > 0;
          bool converged = std::abs(dchisq) < dchisqconverged;
          finished  = ++iter >= m_maxNIter || diverging || converged;
          chisq = newchisq;
          accumulated_chi2 += newchisq;
        }
      }
    }

    const unsigned int number_of_constraints = kalman.getConstraintDim();
    fitpar.addChiSquare(accumulated_chi2, number_of_constraints);

    kalman.updateCovariance(fitpar);
    return status;
  }

  ErrCode Constraint::filterWithReference(FitParams& fitpar, const FitParams& oldState)
  {
    /**
     * We now linearise around the last iteration \alpha (const FitParams& oldState)
     * In this implementation we can no longer linearize non-linear constraints
     * but we ensured by the linearisation around the last state that the step size is small enough
     * so we just use them as if they were linear
     * */
    ErrCode status;
    Projection p(fitpar.getDimensionOfState(), m_dim);
    KalmanCalculator kalman(m_dim, fitpar.getDimensionOfState());

    p.resetProjection();
    status |= project(oldState, p);

    /** here we project the old state and use only the change with respect to the new state
     * instead of the new state in the update . the advantage is more stable fit
     * Downside: non-linear constraints can't be filtered multiple times anymore.
     * */
    p.getResiduals() += p.getH() * (fitpar.getStateVector() - oldState.getStateVector());
    if (!status.failure()) {
      status |= kalman.calculateGainMatrix(
                  p.getResiduals(),
                  p.getH(),
                  fitpar,
                  &p.getV(),
                  1
                );

      if (!status.failure()) {
        kalman.updateState(fitpar);
      }
    }

    const unsigned int number_of_constraints = kalman.getConstraintDim();
    fitpar.addChiSquare(kalman.getChiSquare(), number_of_constraints);

    kalman.updateCovariance(fitpar);
    return status;
  }

  std::string Constraint::name() const
  {
    std::string rc = "unknown constraint!";
    switch (m_type) {
      case beamspot:     rc = "beamspot";   break;
      case beamenergy:   rc = "beamenergy"; break;
      case origin:       rc = "origin"; break;
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
      case helix:        rc = "helix";      break;
      case ntypes:
      case unknown:
        break;
    }
    return rc;
  }
}
