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
#ifndef FRAMEWORK_DATAOBJECTS_MCINITIALPARTICLES_H
#define FRAMEWORK_DATAOBJECTS_MCINITIALPARTICLES_H

#include <TLorentzVector.h>
#include <TLorentzRotation.h>


namespace Belle2 {
  /** This class contains the initial state for the given event.
   * This is not to be confused with the nominal beam parameters but is an event
   * dependent info which contains all implemented smearing effects. It is only
   * set in Montecarlo.
   */
  class MCInitialParticles: public TObject {
  public:
    /** Possible Flags for initial event generation */
    enum EGenerationFlags {
      /** generate initial event in CMS instead of lab */
      c_generateCMS = 1 << 0,
      /** smear energy of HER and LER (but not direction) */
      c_smearBeamEnergy = 1 << 1,
      /** smear direction of HER and LER (but not energy) */
      c_smearBeamDirection = 1 << 2,
      /** smear the full beam momentum (energy and direction) */
      c_smearBeam = c_smearBeamEnergy | c_smearBeamDirection,
      /** smear vertex */
      c_smearVertex = 1 << 3,
      /** smear all */
      c_smearALL = c_smearVertex | c_smearBeam,
    };

    /** Default constructor */
    MCInitialParticles(): m_labToCMS(0), m_generationFlags(0) {}
    /** Free memory of the LorentzRotation if it was created */
    virtual ~MCInitialParticles()
    {
      delete m_labToCMS;
      m_labToCMS = nullptr;
    }

    /** Set the initial event values, i.e. the four momenta of both beams
     * and the vertex
     * @param her 4vector of the high energy beam
     * @param ler 4vector of the low energy beam
     * @param vertex position of the actual collision vertex
     */
    void set(const TLorentzVector& her, const TLorentzVector& ler, const TVector3& vertex)
    {
      m_her = her; m_ler = ler;
      m_vertex = vertex;
      delete m_labToCMS;
      m_labToCMS = nullptr;
    }
    /** Set the High Energy Beam 4-momentum */
    void setHER(const TLorentzVector& her)
    {
      m_her = her;
      delete m_labToCMS;
    }
    /** Set the Low Energy Beam 4-momentum */
    void setLER(const TLorentzVector& ler)
    {
      m_ler = ler;
      delete m_labToCMS;
    }
    /** Set the vertex position */
    void setVertex(const TVector3& vertex)
    {
      m_vertex = vertex;
    }
    /** Get 4vector of the high energy beam */
    const TLorentzVector& getHER() const { return m_her; }
    /** Get 4vector of the low energy beam */
    const TLorentzVector& getLER() const { return m_ler; }
    /** Get the position of the collision */
    const TVector3& getVertex() const { return m_vertex; }
    /** Get the the actual collision energy */
    double getEnergy() const { return (m_her + m_ler).E(); }
    /** Get the invariant mass of the collision */
    double getMass() const { return (m_her + m_ler).M(); }
    /** Return the LorentzRotation to convert from Lab to CMS frame */
    const TLorentzRotation& getLabToCMS() const
    {
      if (!m_labToCMS) calculateBoost(); return *m_labToCMS;
    }

    /** Set the generation flags to be used for event generation */
    void setGenerationFlags(int flags) { m_generationFlags = flags; }
    /** Get the generation flags to be used for event generation */
    int getGenerationFlags() const { return m_generationFlags; }
    /** Check if a certain set of Generation flags is set */
    bool hasGenerationFlags(int flags) { return (m_generationFlags & flags) == flags; }

  private:
    /** Calculate the boost */
    void calculateBoost() const;
    /** HER 4vector */
    TLorentzVector m_her;
    /** LER 4vector */
    TLorentzVector m_ler;
    /** collision position */
    TVector3 m_vertex;
    /** Boost from Lab into CMS. This is calculated on first use and cached
     * but not written to file */
    mutable TLorentzRotation* m_labToCMS; //!transient
    /** Flags to be used when generating events */
    int m_generationFlags;
    /** ROOT Dictionary */
    ClassDef(MCInitialParticles, 1);
  };

  inline void MCInitialParticles::calculateBoost() const
  {
    TLorentzVector beam = m_her + m_ler;
    // Transformation from Lab system to CMS system
    m_labToCMS = new TLorentzRotation(beam.BoostVector());
    // boost HER e- from Lab system to CMS system
    const TLorentzVector electronCMS = (*m_labToCMS) * m_her;
    // now rotate CMS such that incomming e- is parallel to z-axis
    const TVector3 zaxis(0., 0., 1.);
    TVector3 rotaxis = zaxis.Cross(electronCMS.Vect()) * (1. / electronCMS.Vect().Mag());
    double rotangle = TMath::ASin(rotaxis.Mag());
    m_labToCMS->Rotate(-rotangle, rotaxis);
  }

} //Belle2 namespace
#endif // FRAMEWORK_DATAOBJECTS_MCINITIALPARTICLES_H
