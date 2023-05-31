/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/RecoResonance.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

namespace TreeFitter {

  RecoResonance::RecoResonance(Belle2::Particle* particle,
                               const ParticleBase* mother,
                               const ConstraintConfiguration& config
                              ) :
    Composite(particle, mother, config),
    m_massconstraint(false)
  {
    m_massconstraint = std::find(config.m_massConstraintListPDG.begin(), config.m_massConstraintListPDG.end(),
                                 std::abs(m_particle->getPDGCode())) != config.m_massConstraintListPDG.end();
  }

  ErrCode RecoResonance::initParticleWithMother([[gnu::unused]] FitParams& fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoResonance::initMotherlessParticle(FitParams& fitparams)
  {
    const int posindex = posIndex();
    const int momindex = momIndex();

    //quick map for parameters
    int quickindexmap[7];
    for (int i = 0; i < 3; ++i) {
      quickindexmap[i]   = posindex + i;
    }
    for (int i = 0; i < 4; ++i) {
      quickindexmap[i + 3] = momindex + i;
    }

    // copy the 'measurement' -> this overwrites mother position !
    for (int row = 0; row < dimM(); ++row) {
      fitparams.getStateVector()(quickindexmap[row]) = m_params[row];
    }
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoResonance::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::resonance:
        status |= projectComposite(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

  std::string RecoResonance::parname(int index) const
  {
    return ParticleBase::parname(index + 4);
  }

  void RecoResonance::addToConstraintList(constraintlist& list,
                                          int depth) const
  {
    list.push_back(Constraint(this, Constraint::resonance, depth, dimM()));

    if (m_massconstraint) {
      list.push_back(Constraint(this, Constraint::mass, depth, 1, 3));
    }
  }
}
