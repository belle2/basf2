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

#include <framework/particledb/EvtGenParticlePDG.h>
#include <TDatabasePDG.h>
#include <iosfwd>

namespace Belle2 {
  /** Replacement for TDatabasePDG that is filled from EvtGen's evt.pdl. */
  class EvtGenDatabasePDG : public TDatabasePDG {
  public:
    /** Instance method that loads the EvtGen table. */
    static EvtGenDatabasePDG* Instance();
    /** Read EvtGen table */
    void ReadEvtGenTable(const char* filename = nullptr);
    /** Write current database as EvtGen table to a stream */
    void WriteEvtGenTable(std::ostream& out);
    /** Write current database as EvtGen table to a file */
    void WriteEvtGenTable(const char* filename);
    /** override old AddParticle */
    TParticlePDG* AddParticle(const char* name, const char* title, Double_t mass, Bool_t stable, Double_t width,
                              Double_t charge, const char* ParticleClass, Int_t PDGcode, Int_t Anti, Int_t TrackingCode) override
    {
      return AddParticle(name, title, mass, stable, width, charge, ParticleClass, PDGcode, Anti, TrackingCode, 0, 0, 0, 0);
    }
    /** Add new particle with additional attributes. Copy&Paste from original but with more constructor arguments */
    EvtGenParticlePDG* AddParticle(const char* name, const char* title, Double_t mass, Bool_t stable, Double_t width,
                                   Double_t charge, const char* ParticleClass, Int_t PDGcode, Int_t Anti, Int_t TrackingCode,
                                   Double_t Lifetime, Double_t Spin, Double_t maxWidth, Int_t pythiaID);

  private:
    /** singleton. */
    EvtGenDatabasePDG() { }
    /** singleton, make sure there's no copy constructor */
    EvtGenDatabasePDG(const EvtGenDatabasePDG&) = delete;
    /** singleton, make sure there's no assignment operator */
    EvtGenDatabasePDG& operator=(const EvtGenDatabasePDG&) = delete;

    ClassDefOverride(EvtGenDatabasePDG, 1); /**< Replacement for TDatabasePDG that is filled from EvtGen's evt.pdl. */
  };
} //Belle2 namespace
