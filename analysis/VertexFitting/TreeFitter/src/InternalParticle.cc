/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>

#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/V0.h>

using std::vector;

namespace TreeFitter {

  inline bool sortByType(const ParticleBase* lhs, const ParticleBase* rhs)
  {
    int lhstype = lhs->type() ;
    int rhstype = rhs->type() ;
    bool rc = false ;
    if (lhstype == rhstype  &&
        lhstype == ParticleBase::TFParticleType::kRecoTrack) {

      rc =  lhs->particle()->getMomentum().Rho() > rhs->particle()->getMomentum().Rho();
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
                                     const ConstraintConfiguration& config,
                                     bool forceFitAll
                                    ) :
    ParticleBase(particle, mother, &config),// config pointer here to allow final states not to have it
    m_massconstraint(false),
    m_beamconstraint(false),
    m_lifetimeconstraint(false),
    m_isconversion(false),
    m_automatic_vertex_constraining(config.m_automatic_vertex_constraining)
  {
    if (particle) {
      for (auto daughter : particle->getDaughters()) {
        addDaughter(daughter, config, forceFitAll);
      }
    } else {
      B2ERROR("Trying to create an InternalParticle from NULL. This should never happen.");
    }

    m_massconstraint = std::find(config.m_massConstraintListPDG.begin(), config.m_massConstraintListPDG.end(),
                                 std::abs(m_particle->getPDGCode())) != config.m_massConstraintListPDG.end();

    m_beamconstraint = (std::abs(m_particle->getPDGCode()) == config.m_beamConstraintPDG);

    if (!m_automatic_vertex_constraining) {
      // if this is a hadronically decaying resonance it is useful to constraint the decay vertex to its mothers decay vertex.
      //
      m_shares_vertex_with_mother  = std::find(config.m_fixedToMotherVertexListPDG.begin(),
                                               config.m_fixedToMotherVertexListPDG.end(),
                                               std::abs(m_particle->getPDGCode())) != config.m_fixedToMotherVertexListPDG.end() && this->mother();

      // use geo constraint if this particle is in the list to constrain
      m_geo_constraint = std::find(config.m_geoConstraintListPDG.begin(),
                                   config.m_geoConstraintListPDG.end(),
                                   std::abs(m_particle->getPDGCode())) != config.m_geoConstraintListPDG.end()  && this->mother() && !m_shares_vertex_with_mother;
    } else {
      m_shares_vertex_with_mother = this->mother() && m_isStronglyDecayingResonance;
      m_geo_constraint = this->mother() && !m_shares_vertex_with_mother;
    }
  }

  bool InternalParticle::compTrkTransverseMomentum(const RecoTrack* lhs, const RecoTrack* rhs)
  {

    return lhs->particle()->getMomentum().Rho() > rhs->particle()->getMomentum().Rho();
  }

  ErrCode InternalParticle::initMotherlessParticle(FitParams& fitparams)
  {
    ErrCode status ;
    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
    }

    int posindex = posIndex();
    // FIXME update this and check if this position is already initialised
    // lower stream vertices might be better
    if (hasPosition()) {
      fitparams.getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 3, 1>::Zero(3);

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

      if (trkdaughters.size() >= 2) {

        auto v0 = particle()->getV0();
        auto dummy_vertex = ROOT::Math::XYZVector(0, 0, 0);

        bool initWithV0 = false;
        if (v0 && v0->getFittedVertexPosition() != dummy_vertex) {
          auto part_dau1 = particle()->getDaughter(0);
          auto part_dau2 = particle()->getDaughter(1);

          auto recotrack_dau1 = std::find_if(trkdaughters.begin(), trkdaughters.end(),
          [&part_dau1](RecoTrack * rt) { return rt->particle()->getMdstArrayIndex() == part_dau1->getMdstArrayIndex(); });
          auto recotrack_dau2 = std::find_if(trkdaughters.begin(), trkdaughters.end(),
          [&part_dau2](RecoTrack * rt) { return rt->particle()->getMdstArrayIndex() == part_dau2->getMdstArrayIndex(); });

          if (recotrack_dau1 == trkdaughters.end() || recotrack_dau2 == trkdaughters.end()) {
            B2WARNING("V0 daughter particles do not match with RecoTracks.");
          } else {
            double X_V0(v0->getFittedVertexX()), Y_V0(v0->getFittedVertexY()), Z_V0(v0->getFittedVertexZ());
            fitparams.getStateVector()(posindex)     = X_V0;
            fitparams.getStateVector()(posindex + 1) = Y_V0;
            fitparams.getStateVector()(posindex + 2) = Z_V0;

            Belle2::Helix helix1 = v0->getTrackFitResults().first->getHelix();
            Belle2::Helix helix2 = v0->getTrackFitResults().second->getHelix();

            (*recotrack_dau1)->setFlightLength(helix1.getArcLength2DAtXY(X_V0, Y_V0));
            (*recotrack_dau2)->setFlightLength(helix2.getArcLength2DAtXY(X_V0, Y_V0));

            initWithV0 = true;
          }
        }

        if (!initWithV0) {
          std::sort(trkdaughters.begin(), trkdaughters.end(), compTrkTransverseMomentum);

          RecoTrack* dau1 = trkdaughters[0];
          RecoTrack* dau2 = trkdaughters[1];

          Belle2::Helix helix1 = dau1->particle()->getTrackFitResult()->getHelix();
          Belle2::Helix helix2 = dau2->particle()->getTrackFitResult()->getHelix();

          double flt1(0), flt2(0);
          Belle2::B2Vector3D v;
          HelixUtils::helixPoca(helix1, helix2, flt1, flt2, v, m_isconversion);

          fitparams.getStateVector()(posindex)     = v.X();
          fitparams.getStateVector()(posindex + 1) = v.Y();
          fitparams.getStateVector()(posindex + 2) = v.Z();

          dau1->setFlightLength(flt1);
          dau2->setFlightLength(flt2);
        }

      } else if (false && trkdaughters.size() + vtxdaughters.size() >= 2)  {
        // TODO switched off waiting for refactoring of init1 and init2 functions (does not affect performance)
      } else if (mother() && mother()->posIndex() >= 0) {
        const int posindexmother = mother()->posIndex();
        const int dim = m_config->m_originDimension; //TODO access mother
        fitparams.getStateVector().segment(posindex, dim) = fitparams.getStateVector().segment(posindexmother, dim);
      } else {
        /** (0,0,0) is the best guess in any other case */
        fitparams.getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 1, 3>::Zero(3);
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
      const int posindexmom = mother()->posIndex();
      const int dim = m_config->m_originDimension; //TODO access mother?
      fitparams.getStateVector().segment(posindex, dim) = fitparams.getStateVector().segment(posindexmom, dim);
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
        double mass = daughter->particle()->getPDGMass();
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

    for (int imom = 0; imom < 4; ++imom) {
      p.getH()(imom, momindex + imom) = 1;
    }

    for (const auto daughter : m_daughters) {
      const int daumomindex = daughter->momIndex();
      const Eigen::Matrix<double, 1, 3> p3_vec = fitparams.getStateVector().segment(daumomindex, 3);

      // three momentum is easy just subtract the vectors
      p.getResiduals().segment(0, 3) -= p3_vec;

      // energy depends on the parametrisation!
      if (daughter->hasEnergy()) {
        p.getResiduals()(3) -= fitparams.getStateVector()(daumomindex + 3);
        p.getH()(3, daumomindex + 3) = -1; // d/dE -E
      } else {
        // m^2 + p^2 = E^2
        // so
        // E = sqrt(m^2 + p^2)
        const double mass = daughter->particle()->getPDGMass();
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
    return ErrCode(ErrCode::Status::success);
  }


  ErrCode InternalParticle::projectBeamConstraint(const FitParams& fitparams,
                                                  Projection& p) const
  {

    const int momindex = momIndex() ;

    const Eigen::Matrix<double, 4, 1> fitMomE = fitparams.getStateVector().segment(momindex, 4);

    p.getResiduals() = m_config->m_beamMomE - fitMomE;

    for (int row = 0; row < 4; ++row) {
      p.getH()(row, momindex + row) = -1;
    }

    p.getV() =  m_config->m_beamCovariance;

    return ErrCode(ErrCode::Status::success) ;
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
      case Constraint::beam:
        status |= projectBeamConstraint(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }

    return status;
  }

  int InternalParticle::posIndex() const
  {
    // for example B0 and D* can share the same vertex
    return m_shares_vertex_with_mother ? this->mother()->posIndex() : index();
  }

  int InternalParticle::dim() const
  {
    // { x, y, z, tau, px, py, pz, E }
    // the last 4 always exists for composite particles
    // tau index only exist with a vertex and geo constraint
    //
    if (m_shares_vertex_with_mother) { return 4; }
    else if (!m_geo_constraint) { return 7; }
    else { return 8; }
  }

  int InternalParticle::tauIndex() const
  {
    /** only exists if particle is geo constraint and has a mother */
    return m_geo_constraint ? index() + 3 : -1;
  }

  int InternalParticle::momIndex() const
  {
    /** indexing in { x, y, z, tau, px, py, pz, E }
     * but tau is not existing for all InternalParticles
     * */

    if (m_geo_constraint && !m_shares_vertex_with_mother) { return this->index() + 4; }

    if (m_shares_vertex_with_mother) { return this->index(); }

    if (!m_geo_constraint) {return index() + 3 ;}

    // this will crash the initialisation
    return -1;
  }

  bool InternalParticle::hasPosition() const
  {
    //  does this particle have a position index (decayvertex)
    //  true in any case
    return true;
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
    if (m_geo_constraint) {
      assert(m_config);
      const int dim = m_config->m_originDimension == 2 && std::abs(m_particle->getPDGCode()) == m_config->m_headOfTreePDG ? 2 : 3;
      list.push_back(Constraint(this, Constraint::geometric, depth, dim, 3));
    }
    if (m_massconstraint) {
      list.push_back(Constraint(this, Constraint::mass, depth, 1, 3));
    }
    if (m_beamconstraint) {
      assert(m_config);
      list.push_back(Constraint(this, Constraint::beam, depth, 4, 3));
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
