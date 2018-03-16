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
#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>
#include <framework/logging/Logger.h>


using std::vector;

namespace TreeFitter {

  extern int vtxverbose ;
  extern std::vector<int> massConstraintList ;

  inline bool sortByType(const ParticleBase* lhs, const ParticleBase* rhs)
  {
    int lhstype = lhs->type() ;
    int rhstype = rhs->type() ;
    bool rc = false ;
    if (lhstype == rhstype  &&
        lhstype == ParticleBase::kRecoTrack) {

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
    m_lifetimeconstraint(false)
  {

    if (particle) {
      for (auto daughter : particle->getDaughters()) {
        addDaughter(daughter, forceFitAll);
      }
    } else {
      B2ERROR("Trying to create an InternalParticle from NULL. This should never happen.");
    }

    //FT: Need a method to flag these individually for each particle. Currently I use the PDG code, but I want to switch to DecayDescriptors
    m_massconstraint     = false;
    int pdgcode; //JFK 0 for beamspot
    if (particle) {
      pdgcode = particle->getPDGCode();
    } else {
      pdgcode = 0;
    }

    // JFK:: replace with is size > 0 or contructor flag which is even better
    if (std::find(massConstraintList.begin(), massConstraintList.end(), pdgcode) != massConstraintList.end()) {
      m_massconstraint = true;
    }

    //FT: These aren't available yet
    m_lifetimeconstraint = false;
    m_isconversion = false;
  }

  bool compTrkTransverseMomentum(const RecoTrack* lhs, const RecoTrack* rhs)
  {
    return lhs->particle()->getMomentum().Perp() > rhs->particle()->getMomentum().Perp();
  }

  ErrCode InternalParticle::initMotherlessParticle(FitParams* fitparams)
  {
    ErrCode status ;
    int posindex = posIndex();

    // logic check: we do not want to call this routine for resonances.
    assert(hasPosition());

    // Start with origin
    fitparams->getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 3, 1>::Zero(3);

    // Step 1: pre-initialization of all daughters
    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
    }
    // Step 2: initialize the vertex. if we are lucky, we had a
    // recoresonant daughter, and we are already done.
    // (vertex already exists)
    if (fitparams->getStateVector()(posindex)  == 0 &&
        fitparams->getStateVector()(posindex + 1) == 0 &&
        fitparams->getStateVector()(posindex + 2) == 0) {

      //otherwise, composites are initialized with a vertex at (0,0,0); if it's different, they were already vertexed; use that.
      TVector3 vtx = getBasf2Particle()->getVertex();
      if (vtx.Mag()) { //if it's not zero
        fitparams->getStateVector()(posindex) = vtx.X();
        fitparams->getStateVector()(posindex + 1) = vtx.Y();
        fitparams->getStateVector()(posindex + 2) = vtx.Z();
      } else {

        // Case B: the hard way ... use the daughters to estimate the
        // vertex. First we check if there are sufficient tracks
        // attached to this vertex. If so, estimate the poca of the
        // two tracks with the highest momentum. This will work for
        // the majority of the cases. If there are not sufficient
        // tracks, add the composites and take the two with the best
        // doca.

        // create a vector with all daughters that constitute a
        // 'trajectory' (ie tracks, composites and daughters of
        // resonances.)
        std::vector<ParticleBase*> alldaughters;
        ParticleBase::collectVertexDaughters(alldaughters, posindex);

        // select daughters that are either charged, or have an initialized vertex
        std::vector<ParticleBase*> vtxdaughters;

        vector<RecoTrack*> trkdaughters;
        for (auto daughter : alldaughters) {
          if (daughter->type() == ParticleBase::kRecoTrack) {
            trkdaughters.push_back(static_cast<RecoTrack*>(daughter));
          } else if (daughter->hasPosition()
                     && fitparams->getStateVector()(daughter->posIndex()) != 0) {
            vtxdaughters.push_back(daughter);
          }
        }

        if (trkdaughters.size() >= 2) {
          // sort in pT. not very efficient, but it works.
          if (trkdaughters.size() > 2) {
            std::sort(trkdaughters.begin(), trkdaughters.end(), compTrkTransverseMomentum);
          }

          // now, just take the first two ...
          RecoTrack* dau1 = trkdaughters[0];
          RecoTrack* dau2 = trkdaughters[1];

          //Using pion hypothesis is fine for initialization purposes
          Belle2::Helix helix1 = dau1->particle()->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::pion)->getHelix();
          Belle2::Helix helix2 = dau2->particle()->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::pion)->getHelix();

          double flt1(0), flt2(0);

          TVector3 v;
          HelixUtils::helixPoca(helix1, helix2, flt1, flt2, v, m_isconversion);


          fitparams->getStateVector()(posindex)     = -v.x();
          fitparams->getStateVector()(posindex + 1) = -v.y();
          fitparams->getStateVector()(posindex + 2) = -v.z();

          dau1->setFlightLength(flt1);
          dau2->setFlightLength(flt2);

          /** FIXME temporarily disabled */
        } else if (false && trkdaughters.size() + vtxdaughters.size() >= 2)  {
          // that's unfortunate: no enough charged tracks from this
          // vertex. need all daughters. create trajectories and use
          // normal TrkPoca.

          //JFK: FIXME 2017-09-25
          //B2DEBUG("Internal particle l181 track + other daughter::Is this implementd?");
          B2DEBUG(80, "VtkInternalParticle: Low # charged track initializaton. To be implemented!!");

        } else if (mother() && mother()->posIndex() >= 0) {
          // let's hope the mother was initialized
          int posindexmother = mother()->posIndex();

          fitparams->getStateVector().segment(posindex, 3) = fitparams->getStateVector().segment(posindexmother, 3);

        } else {
          // something is wrong!
          //    BtaPrintTree treeprinter ;
          B2ERROR("There are not sufficient geometric constraints to fit "
                  << "this decay tree. Perhaps you should add a beam constraint. "
                  << " This happend for a " << this->name() << " candidate.");
          status |= ErrCode::badsetup;
        }
      }
    }

    // step 3: do the post initialization step of all daughters
    for (auto daughter :  m_daughters) {
      daughter->initParticleWithMother(fitparams);
    }

    // step 4: initialize the momentum by adding up the daughter 4-vectors
    initMomentum(fitparams);
    return status;
  }



  ErrCode InternalParticle::initParticleWithMother(FitParams* fitparams)
  {
    // FIX ME: in the unfortunate case (the B-->D0K*- above) that our
    // vertex is still the origin, we copy the mother vertex.
    int posindex = posIndex();
    int posindexmom = 0;

    if (hasPosition() &&
        mother() &&
        fitparams->getStateVector()(posindex) == 0 &&
        fitparams->getStateVector()(posindex + 1) == 0 && \
        fitparams->getStateVector()(posindex + 2) == 0) {

      posindexmom = mother()->posIndex();
      fitparams->getStateVector().segment(posindex , 3) = fitparams->getStateVector().segment(posindexmom, 3);

    }

    return initTau(fitparams);
  }

  ErrCode InternalParticle::initMomentum(FitParams* fitparams) const
  {
    int momindex = momIndex();
    fitparams->getStateVector().segment(momindex, 4) = Eigen::Matrix<double, 4, 1>::Zero(4);

    int daumomindex = 0, maxrow = 0;
    double e2 = 0, mass = 0;

    for (auto daughter : m_daughters) {

      daumomindex = daughter->momIndex();
      maxrow = daughter->hasEnergy() ? 4 : 3;

      e2 = fitparams->getStateVector().segment(daumomindex, maxrow).squaredNorm();
      fitparams->getStateVector().segment(momindex, maxrow) += fitparams->getStateVector().segment(daumomindex, maxrow);

      if (maxrow == 3) {
        mass = daughter->pdgMass();
        fitparams->getStateVector()(momindex + 3) += std::sqrt(e2 + mass * mass);
      }

    }
    return ErrCode::success;
  }

  ErrCode InternalParticle::initCovariance(FitParams* fitparams) const
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
    // first add the mother
    int momindex = momIndex();

    p.getResiduals().segment(0, 4) = fitparams.getStateVector().segment(momindex, 4);
    for (int imom = 0; imom < 4; ++imom) {
      p.getH()(imom, momindex + imom) = 1;
    }

    // now add the daughters
    const double posprecision = 1e-4; // 1mu

    for (const auto daughter : m_daughters) {

      int dautauindex = 0, daumomindex = 0, maxrow = 0;
      double mass = 0, e2 = 0, px = 0, py = 0, energy = 0, tau = 0, lambda = 0, px0 = 0, py0 = 0, pt0 = 0, sinlt = 0, coslt = 0;

      dautauindex = daughter->tauIndex();
      daumomindex = daughter->momIndex();
      mass = daughter->pdgMass();
      maxrow = daughter->hasEnergy() ? 4 : 3;
      e2 = mass * mass;

      for (int imom = 0; imom < maxrow; ++imom) {
        px = fitparams.getStateVector()(daumomindex + imom);
        e2 += px * px;
        p.getResiduals()(imom) += -px;
        p.getH()(imom, daumomindex + imom) = -1;
      }

      if (maxrow == 3) {
        // treat the energy for particles that are parameterized with p3
        energy = sqrt(e2);
        p.getResiduals()(3) += -energy;

        for (int jmom = 0; jmom < 3; ++jmom) {
          px = fitparams.getStateVector()(daumomindex + jmom);
          p.getH()(3, daumomindex + jmom) = -px / energy;
        }

        //FIXME switched off linear approximation should be fine the stuff below uses a helix...
      } else if (false && dautauindex >= 0 && daughter->charge() != 0) {

        tau =  fitparams.getStateVector()(dautauindex);
        lambda = bFieldOverC() * daughter->charge();

        px0 = fitparams.getStateVector()(daumomindex);
        py0 = fitparams.getStateVector()(daumomindex + 1);
        pt0 = sqrt(px0 * px0 + py0 * py0);

        if (fabs(pt0 * lambda * tau * tau) > posprecision) {
          sinlt = sin(lambda * tau);
          coslt = cos(lambda * tau);
          px = px0 * coslt - py0 * sinlt;
          py = py0 * coslt + px0 * sinlt;

          p.getResiduals()(0) += px0 - px;
          p.getResiduals()(1) += py0 - py;

          p.getH()(0, daumomindex) += 1 - coslt  ;
          p.getH()(0, daumomindex + 1) += sinlt      ;
          p.getH()(0, dautauindex) += lambda * py;
          p.getH()(1, daumomindex) -= sinlt      ;
          p.getH()(1, daumomindex + 1) += 1 - coslt  ;
          p.getH()(1, dautauindex) -= lambda * px;
        }
      }

    }

    return ErrCode::success;
  }


  ErrCode InternalParticle::projectConstraint(const Constraint::Type type,
                                              const FitParams& fitparams,
                                              Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::mass:
      case Constraint::massEnergy:
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

  void InternalParticle::addToConstraintList(constraintlist& list,
                                             int depth) const
  {

    for (auto daughter : m_daughters) {
      daughter->addToConstraintList(list, depth - 1);
    }

    // the lifetime constraint
    if (tauIndex() >= 0 && m_lifetimeconstraint) {
      list.push_back(Constraint(this, Constraint::lifetime, depth, 1));
    }

    // the kinematic constraint
    if (momIndex() >= 0) {
      list.push_back(Constraint(this, Constraint::kinematic, depth, 4));
    }

    // the geometric constraint
    if (mother() && tauIndex() >= 0) {
      list.push_back(Constraint(this, Constraint::geometric, depth, 3, 5));
    }

    // the mass constraint
    if (m_massconstraint) {
      if (!m_isconversion) {
        list.push_back(Constraint(this, Constraint::mass, depth, 1, 10));
      } else {
        list.push_back(Constraint(this, Constraint::conversion, depth, 1, 3));
      }
    }
  }


  std::string InternalParticle::parname(int thisindex) const
  {
    int id = thisindex;
    // skip the lifetime parameter if there is no mother
    if (!mother() && id >= 3) {++id;}
    return ParticleBase::parname(id);
  }

  void InternalParticle::forceP4Sum(FitParams& fitparams) const //FT: this needs double checking
  {
    // because things are not entirely linear, p4 is not exactly
    // conserved at the end of fits that include mass
    // constraints. this routine is called after the tree is fitted to
    // ensure that p4 'looks' conserved.

    // first the daughters
    for (const auto daughter : m_daughters) {
      daughter->forceP4Sum(fitparams);
    }

    int momindex = momIndex();
    if (momindex > 0) {

      Projection p(fitparams.getDimensionOfState(), 4);
      projectKineConstraint(fitparams, p);
      fitparams.getStateVector().segment(momindex, 4) -= p.getResiduals().segment(momindex, 4);
    }
  }

}

