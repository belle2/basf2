/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TParticlePDG.h>

namespace Belle2 {
  /** Helper class for setting additional TParticlePDG members and storing the
   * ones it doesn't have yet. ROOT defines the members for lifteme a but has
   * no way to set them, so let's make a class which has. Also we need a few
   * additional members to represent all data in evtgen/evt.pdl */
  class EvtGenParticlePDG : public TParticlePDG {
  public:
    /** Construct with all values */
    EvtGenParticlePDG(const char* name, const char* title, Double_t mass, Bool_t stable, Double_t width,
                      Double_t charge, const char* ParticleClass, Int_t PDGcode, Int_t Anti, Int_t TrackingCode,
                      Double_t Lifetime, Double_t Spin, Double_t maxWidth, int pythiaID):
      TParticlePDG(name, title, mass, stable, width, charge, ParticleClass, PDGcode, Anti, TrackingCode),
      m_maxWidth(maxWidth), m_pythiaID(pythiaID)
    {
      // override members in base class
      fLifetime = Lifetime;
      fSpin = Spin;
    }
    /** Return max width (max_Dm in evt.pdl) */
    Double_t MaxWidth() const { return m_maxWidth; }
    /** Return pythia id of this particle */
    Int_t PythiaID() const { return m_pythiaID; }
    /** Return a string repesentation of this particle to be used when printing the particle in python */
    std::string __repr__() const;
  protected:
    /** max width of the particle (max_Dm in evt.pdl) */
    Double_t m_maxWidth{0};
    /** pythia id of the particle */
    Int_t m_pythiaID{0};
    /** ROOT Dictionary */
    ClassDef(EvtGenParticlePDG, 1);
  };

} //Belle2 namespace
