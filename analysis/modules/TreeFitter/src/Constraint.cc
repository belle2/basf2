/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <iomanip>
#include <framework/logging/Logger.h>
//
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/Constraint.h>
#include <analysis/modules/TreeFitter/KalmanCalculator.h>

namespace TreeFitter {

  extern int vtxverbose ;

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

    //JFK FIXME will this code ever be reached??

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
    // this one will be overloaded by the MergedConstraint
    return m_node->projectConstraint(m_type, fitpar, p);
  }

  // JFK: no longer const Mon 04 Sep 2017 05:06:10 AM CEST
  ErrCode Constraint::filter(FitParams* fitpar)
  {
    ErrCode status;
    if (m_type <= Constraint::unknown || m_type >= Constraint::ntypes) {
      std::cout << "VtkConstraint: unknown constraint: " << m_type << std::endl;
      status |= ErrCode::badsetup;
    } else if (m_type != merged && !m_node) {
      std::cout << "VtkConstraint: filter constraint without a node" << std::endl;
      status |= ErrCode::badsetup;
    } else {
      if (vtxverbose >= 3) { std::cout << "filtering "  ; print() ;}
      // save the unfiltered ('predicted') parameters. we need to
      // store them if we want to iterate constraints.
      const CLHEP::HepVector* pred(0);

      if (m_maxNIter > 1) {
        pred = new CLHEP::HepVector(fitpar->par());
      }

      Projection p(fitpar->dim(), m_dim) ;
      KalmanCalculator kalman ;
      double chisq(0) ;
      int iter(0) ;
      bool finished(false) ;
      while (!finished && !status.failure()) {
        p.reset();

        status |= project(*fitpar, p);

        if (!status.failure()) {
          status |= kalman.init(p.r(), p.H(), fitpar, &p.V());

          if (!status.failure()) {

            if (iter == 0 || !pred) {
              kalman.updatePar(fitpar);
            } else {
              kalman.updatePar(*pred, fitpar);
            }
            const double dchisqconverged = 0.001;
            double newchisq = kalman.chisq();
            double dchisq = newchisq - chisq;
            bool diverging = iter > 0 && dchisq > 0;
            bool converged = fabs(dchisq) < dchisqconverged;
            finished  = ++iter >= m_maxNIter || diverging || converged;

            if (vtxverbose >= 3) {
              std::cout << "chi2,niter: "
                        << iter << " " << std::setprecision(7)
                        << std::setw(12) << chisq << " "
                        << std::setw(12) << newchisq << " "
                        << std::setw(12) << dchisq << " "
                        << diverging << " "
                        << converged << " "
                        << status << std::endl;
            }
            chisq = newchisq;
          }
        }
      }
      //std::cout << "### Constraints kalman(!) chi2 : " << kalman.chisq() << " dim " << m_dim << " for " << this->name() << std::endl;


      // JFK: the chi2 sum of fitpars will be used as the one for the newton iteration
      // this also serves as the chi2 for the final PVal calculation
      // it is reset in decay chain before the loop over the constraints Wed 06 Sep 2017 10:07:53 AM CEST
      fitpar->addChiSquare(kalman.chisq(), kalman.getConstraintDim());
      kalman.updateCov(fitpar);

      if (!status.failure()) {

        if (m_nHidden > 0) {
          fitpar->addChiSquare(0, -m_nHidden);
        }
      }

      if (pred) {delete pred;}

      if (vtxverbose >= 4 && m_node && m_node->mother()) {
        m_node->mother()->print(fitpar);
      }
    }
    return status ;
  }


//FT: new filter function using a fixed reference from last Kalman iteration
// // JFK: no longer const Mon 04 Sep 2017 05:06:39 AM CEST
  ErrCode Constraint::filter(FitParams* fitpar, const FitParams* reference)

  {

    // filter but linearize around reference
    ErrCode status ;

    double chisq = 1e10;

    if (m_type <= Constraint::unknown || m_type >= Constraint::ntypes) {
      std::cout << "VtkConstraint: unknown constraint: " << m_type
                << std::endl;
      status |= ErrCode::badsetup;
    } else if (m_type != merged && !m_node) {
      std::cout << "VtkConstraint: filter constraint without a node"
                << std::endl;
      status |= ErrCode::badsetup;
    } else {
      if (vtxverbose >= 3) { std::cout << "filtering "; print();}
      // project using the reference
      Projection p(fitpar->dim(), m_dim);
      status = project(*reference, p);
      // now update the residual

      //FT: test output
      //std::cout << "Delta p.r() = " << p.H() * (fitpar->par() - reference->par()) << std::endl;
      p.r() += p.H() * (fitpar->par() - reference->par());

      // now call the Kalman update as usual
      KalmanCalculator kalman;
      status |= kalman.init(p.r(), p.H(), fitpar, &p.V());
      kalman.updatePar(fitpar);

      kalman.updateCov(fitpar);

      chisq = kalman.chisq();

      if (fitpar->cov(m_node->index() + 1) < 0 || kalman.chisq() < 0 || std::isnan(kalman.chisq())) {
        status |= ErrCode::filtererror;
      }
    }
    if (status.failure() && vtxverbose >= 1) {
      std::cout << "error filtering constraint: "
                << name() << " " << status << std::endl;
    }

    return status;
  }

  void Constraint::print(std::ostream& os) const
  {
    os << m_node->index() << " "
       << m_node->name().c_str() << " "
       << name().c_str() << " "
       << m_type << " " << m_depth << std::endl;
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
