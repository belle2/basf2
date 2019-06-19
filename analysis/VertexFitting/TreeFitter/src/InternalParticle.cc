/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>

#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>
#include <framework/logging/Logger.h>
using std::vector;

namespace TreeFitter {

  inline bool sortByType(const ParticleBase* lhs, const ParticleBase* rhs)
  {
    int lhstype = lhs->type() ;
    int rhstype = rhs->type() ;
    bool rc = false ;
    if (lhstype == rhstype  &&
        lhstype == ParticleBase::TFParticleType::kRecoTrack) {

      rc =  lhs->particle()->getMomentum().Perp() > rhs->particle()->getMomentum().Perp();
    } else if (lhs->particle() && rhs->particle() && lhs->particle()->getNDaughters() > 0 &&
               rhs->particle()->getNDaughters() > 0) {
      rc = lhs->nFinalChargedCandidates() > rhs->nFinalChargedCandidates();
    } else {
      rc = lhstype < rhstype;
    }
    return rc;
  }

  InternalParticle::InternalParticle(Belle2::Particle* particle,
                                     const ParticleBase* mother,
                                     bool forceFitAll) :
    ParticleBase(particle, mother),
    m_massconstraint(false),
    m_lifetimeconstraint(false),
    m_isconversion(false)
  {

    if (particle) {
      for (auto daughter : particle->getDaughters()) {
        addDaughter(daughter, forceFitAll);
      }
    } else {
      B2ERROR("Trying to create an InternalParticle from NULL. This should never happen.");
    }
  }

  bool InternalParticle::compTrkTransverseMomentum(const RecoTrack* lhs, const RecoTrack* rhs)
  {

    return lhs->particle()->getMomentum().Perp() > rhs->particle()->getMomentum().Perp();
  }

  ErrCode InternalParticle::initMotherlessParticle(FitParams& fitparams)
  {
    ErrCode status ;
    int posindex = posIndex();

    assert(hasPosition());

    fitparams.getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 3, 1>::Zero(3);

    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
    }

    if (fitparams.getStateVector()(posindex)  == 0 &&
        fitparams.getStateVector()(posindex + 1) == 0 &&
        fitparams.getStateVector()(posindex + 2) == 0) {

      TVector3 vtx = particle()->getVertex();
      if (vtx.Mag()) { //if it's not zero
        fitparams.getStateVector()(posindex) = vtx.X();
        fitparams.getStateVector()(posindex + 1) = vtx.Y();
        fitparams.getStateVector()(posindex + 2) = vtx.Z();
      } else {

        std::vector<ParticleBase*> alldaughters;
        ParticleBase::collectVertexDaughters(alldaughters, posindex);

        std::vector<ParticleBase*> vtxdaughters;

        vector<RecoTrack*> trkdaughters;
        for (auto daughter : alldaughters) {
          if (daughter->type() == ParticleBase::TFParticleType::kRecoTrack) {
            trkdaughters.push_back(static_cast<RecoTrack*>(daughter));
          } else if (daughter->hasPosition()
                     && fitparams.getStateVector()(daughter->posIndex()) != 0) {
            vtxdaughters.push_back(daughter);
          }
        }

        TVector3 v;

        if (trkdaughters.size() >= 2) {
          std::sort(trkdaughters.begin(), trkdaughters.end(), compTrkTransverseMomentum);

          RecoTrack* dau1 = trkdaughters[0];
          RecoTrack* dau2 = trkdaughters[1];

          Belle2::Helix helix1 = dau1->particle()->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::pion)->getHelix();
          Belle2::Helix helix2 = dau2->particle()->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::pion)->getHelix();

          double flt1(0), flt2(0);
          HelixUtils::helixPoca(helix1, helix2, flt1, flt2, v, m_isconversion);

          fitparams.getStateVector()(posindex)     = v.x();
          fitparams.getStateVector()(posindex + 1) = v.y();
          fitparams.getStateVector()(posindex + 2) = v.z();

          dau1->setFlightLength(flt1);
          dau2->setFlightLength(flt2);

          /** temporarily disabled */
        } else if (false && trkdaughters.size() + vtxdaughters.size() >= 2)  {
          B2DEBUG(12, "VtkInternalParticle: Low # charged track initializaton. To be implemented!!");

        } else if (mother() && mother()->posIndex() >= 0) {
          int posindexmother = mother()->posIndex();

          fitparams.getStateVector().segment(posindex, 3) = fitparams.getStateVector().segment(posindexmother, 3);

        } else {
          /** (0,0,0) is the best guess in any other case */
          fitparams.getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 1, 3>::Zero(3);
        }
      }
    }

    for (auto daughter :  m_daughters) {
      daughter->initParticleWithMother(fitparams);
    }

    initMomentum(fitparams);
    return status;
  }

  ErrCode InternalParticle::initParticleWithMother(FitParams& fitparams)
  {
    int posindex = posIndex();

    if (hasPosition() &&
        mother() &&
        fitparams.getStateVector()(posindex) == 0 &&
        fitparams.getStateVector()(posindex + 1) == 0 && \
        fitparams.getStateVector()(posindex + 2) == 0) {
      int posindexmom = mother()->posIndex();
      fitparams.getStateVector().segment(posindex , 3) = fitparams.getStateVector().segment(posindexmom, 3);
    }
    return initTau(fitparams);
  }

  ErrCode InternalParticle::initMomentum(FitParams& fitparams) const
  {
    int momindex = momIndex();
    fitparams.getStateVector().segment(momindex, 4) = Eigen::Matrix<double, 4, 1>::Zero(4);

    for (auto daughter : m_daughters) {
      int daumomindex = daughter->momIndex();
      int maxrow = daughter->hasEnergy() ? 4 : 3;

      double e2 = fitparams.getStateVector().segment(daumomindex, maxrow).squaredNorm();
      fitparams.getStateVector().segment(momindex, maxrow) += fitparams.getStateVector().segment(daumomindex, maxrow);

      if (maxrow == 3) {
        double mass = daughter->pdgMass();
        fitparams.getStateVector()(momindex + 3) += std::sqrt(e2 + mass * mass);
      }
    }
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode InternalParticle::initCovariance(FitParams& fitparams) const
  {
    ErrCode status;
    ParticleBase::initCovariance(fitparams);
    for (auto daughter : m_daughters) {
      status |= daughter->initCovariance(fitparams);
    }
    return status;
  }


  ErrCode InternalParticle::projectKineConstraint(const FitParams& fitparams,
                                                  Projection& p) const
  {
    const int momindex = momIndex();

    // `this` always has an energy row
    p.getResiduals().segment(0, 4) = fitparams.getStateVector().segment(momindex, 4);
    //p.getResiduals().segment(0, 3) = fitparams.getStateVector().segment(momindex, 3);
    //const double m = this->pdgMass();
    //const double p_2 = fitparams.getStateVector().segment(momindex, 3).squaredNorm();
    //const double e = std::sqrt(m*m + p_2);
    //p.getResiduals()(3) = e;

    for (int imom = 0; imom < 4; ++imom) {
      p.getH()(imom, momindex + imom) = 1;
      //p.getH()(3, momindex + imom) = -fitparams.getStateVector()(momindex+imom) / e;
    }

    for (const auto daughter : m_daughters) {
      const int daumomindex = daughter->momIndex();
      const Eigen::Matrix<double, 1, 3> p3_vec = fitparams.getStateVector().segment(daumomindex, 3);

      // three momentum is easy just substract the vectors
      p.getResiduals().segment(0, 3) -= p3_vec;

      // energy depends on the parametrisation!
      if (daughter->hasEnergy()) {
        p.getResiduals()(3) -= fitparams.getStateVector()(daumomindex + 3);
        p.getH()(3, daumomindex + 3) = -1; // d/dE -E
      } else {
        // m^2 + p^2 = E^2
        // so
        // E = sqrt(m^2 + p^2)
        const double mass = daughter->pdgMass();
        const double p2 = p3_vec.squaredNorm();
        const double energy = std::sqrt(mass * mass + p2);
        p.getResiduals()(3) -= energy;

        for (unsigned i = 0; i < 3; ++i) {
          // d/dpx_i sqrt(m^2 + p^2)
          p.getH()(3, daumomindex + i) = -1 * p3_vec(i) / energy;
        }
      }

      // this has to be in any case
      // d/dp_i p_i
      for (unsigned i = 0; i < 3; ++i) {
        p.getH()(i, daumomindex + i) = -1;
      }
    }

    //int dautauindex = daughter->tauIndex();
    //int daumomindex = daughter->momIndex();
    //double mass = daughter->pdgMass();
    //int maxrow = daughter->hasEnergy() ? 4 : 3;
    //double e2 = mass * mass;

    //double px = 0, py = 0, tau = 0, lambda = 0, px0 = 0, py0 = 0, pt0 = 0, sinlt = 0, coslt = 0;

    //for (int imom = 0; imom < maxrow; ++imom) {
    //  px = fitparams.getStateVector()(daumomindex + imom);
    //  e2 += px * px;
    //  p.getResiduals()(imom) += -px;
    //  p.getH()(imom, daumomindex + imom) = -1;
    //}

    //if (maxrow == 3) {
    //  double energy = sqrt(e2);
    //  p.getResiduals()(3) += -energy;

    //  for (int jmom = 0; jmom < 3; ++jmom) {
    //    px = fitparams.getStateVector()(daumomindex + jmom);
    //    p.getH()(3, daumomindex + jmom) = -px / energy;
    //  }

    //} else if (false && dautauindex >= 0 && daughter->charge() != 0) {
    //  tau =  fitparams.getStateVector()(dautauindex);
    //  lambda = bFieldOverC() * daughter->charge();

    //  px0 = fitparams.getStateVector()(daumomindex);
    //  py0 = fitparams.getStateVector()(daumomindex + 1);
    //  pt0 = sqrt(px0 * px0 + py0 * py0);

    //  if (fabs(pt0 * lambda * tau * tau) > posprecision) {
    //    sinlt = sin(lambda * tau);
    //    coslt = cos(lambda * tau);
    //    px = px0 * coslt - py0 * sinlt;
    //    py = py0 * coslt + px0 * sinlt;

    //    p.getResiduals()(0) += px0 - px;
    //    p.getResiduals()(1) += py0 - py;

    //    p.getH()(0, daumomindex) += 1 - coslt  ;
    //    p.getH()(0, daumomindex + 1) += sinlt      ;
    //    p.getH()(0, dautauindex) += lambda * py;
    //    p.getH()(1, daumomindex) -= sinlt      ;
    //    p.getH()(1, daumomindex + 1) += 1 - coslt  ;
    //    p.getH()(1, dautauindex) -= lambda * px;
    //  }
    //}
    //}
    return ErrCode(ErrCode::Status::success);
  }


  ErrCode InternalParticle::projectConstraint(const Constraint::Type type,
                                              const FitParams& fitparams,
                                              Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::mass:
        status |= projectMassConstraint(fitparams, p);
        break;
      case Constraint::geometric:
        status |= projectGeoConstraint(fitparams, p);
        break;
      case Constraint::kinematic:
        status |= projectKineConstraint(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }

    return status;
  }

  int InternalParticle::dim() const
  {
    /** if this particle is geo constraint the dimension is 8, because tau has to be extracted */
    return mother() && !isAResonance(m_particle) ? 8 : 7 ;
  }

  int InternalParticle::tauIndex() const
  {
    /** only exists if particle is geo cosntraint */
    return mother() ? index() + 3 : -1;
  }

  int InternalParticle::momIndex() const
  {
    /** indexing in { x, y, z, tau, px, py, pz, E }
     * but tau is not existing for all InternalParticles
     * */
    return mother() ? index() + 4 : index() + 3 ;
  }

  void InternalParticle::addToConstraintList(constraintlist& list,
                                             int depth) const
  {

    for (auto daughter : m_daughters) {
      daughter->addToConstraintList(list, depth - 1);
    }
    if (tauIndex() >= 0 && m_lifetimeconstraint) {
      list.push_back(Constraint(this, Constraint::lifetime, depth, 1));
    }
    if (momIndex() >= 0) {
      list.push_back(Constraint(this, Constraint::kinematic, depth, 4, 3));
    }
    if (mother() && tauIndex() >= 0) {
      list.push_back(Constraint(this, Constraint::geometric, depth, 3, 3));
    }

    if (std::find(TreeFitter::massConstraintListPDG.begin(), TreeFitter::massConstraintListPDG.end(),
                  std::abs(particle()->getPDGCode())) != TreeFitter::massConstraintListPDG.end()) {
      list.push_back(Constraint(this, Constraint::mass, depth, 1, 3));
    }


    //FIXME needs mother and has to change index of this
    const bool pin_vertex_to_mother_vertex = std::find(TreeFitter::fixedToMotherVertexListPDG.begin(),
                                                       TreeFitter::fixedToMotherVertexListPDG.end(),
                                                       std::abs(particle()->getPDGCode())) != TreeFitter::fixedToMotherVertexListPDG.end();

    if (pin_vertex_to_mother_vertex) {
      list.push_back(Constraint(this, Constraint::mass, depth, 1, 3));
    }

    // use geo constraint if this particle is in the list to cosntrain
    const bool geo_constrain_this = std::find(TreeFitter::geoConstraintListPDG.begin(),
                                              TreeFitter::geoConstraintListPDG.end(),
                                              std::abs(particle()->getPDGCode())) != TreeFitter::geoConstraintListPDG.end();

    // FIXME tauindex
    if (geo_constrain_this && mother() && tauIndex() >= 0) {
      list.push_back(Constraint(this, Constraint::geometric, depth, 3, 3));
    }

  }

  std::string InternalParticle::parname(int thisindex) const
  {
    int id = thisindex;
    if (!mother() && id >= 3) {++id;}
    return ParticleBase::parname(id);
  }

  void InternalParticle::forceP4Sum(FitParams& fitparams) const
  {
    for (const auto daughter : m_daughters) {
      daughter->forceP4Sum(fitparams);
    }
    const int momindex = momIndex();
    if (momindex > 0) {
      const int dim = hasEnergy() ? 4 : 3;
      Projection p(fitparams.getDimensionOfState(), dim);
      projectKineConstraint(fitparams, p);
      fitparams.getStateVector().segment(momindex, dim) -= p.getResiduals().segment(0, dim);
    }
  }

}

