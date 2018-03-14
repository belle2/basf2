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
    MCInitialParticles(): TObject() {}
    /** Copy constructor */
    MCInitialParticles(const MCInitialParticles& b): TObject(), m_her(b.m_her), m_ler(b.m_ler),
      m_vertex(b.m_vertex), m_generationFlags(b.m_generationFlags) {}
    /** Assignment operator */
    MCInitialParticles& operator=(const MCInitialParticles& b)
    {
      m_her = b.m_her; m_ler = b.m_ler; m_vertex = b.m_vertex;
      m_generationFlags = b.m_generationFlags;
      resetBoost();
      return *this;
    }
    /** Equality operator */
    bool operator==(const MCInitialParticles& b) const
    {
      // FIXME: Ah, bloody hell. sin(x) returns slightly different values on
      // different platforms in some cases so we cannot just do an equality
      // comparison. We need to do this more elegantly, this is just for
      // testing if it solves all problems
#if defined(MCP_DBL_CMP) || defined(MCP_VEC3_CMP) || defined(MCP_VEC4_CMP)
#error Macro already defined, cannot continue
#endif
#define MCP_DBL_CMP(a,b,x) ((a.x()==b.x())||(std::abs(a.x()-b.x())<1e-10))
#define MCP_VEC3_CMP(a,b) (MCP_DBL_CMP(a,b,X) && MCP_DBL_CMP(a,b,Y) && MCP_DBL_CMP(a,b,Z))
#define MCP_VEC4_CMP(a,b) (MCP_VEC3_CMP(a,b) && MCP_DBL_CMP(a,b,E))
      return MCP_VEC4_CMP(m_her, b.m_her) && MCP_VEC4_CMP(m_ler, b.m_ler) && MCP_VEC3_CMP(m_vertex, b.m_vertex) &&
             m_generationFlags == b.m_generationFlags;
#undef MCP_DBL_CMP
#undef MCP_VEC3_CMP
#undef MCP_VEC4_CMP
    }

    /** Free memory of the LorentzRotation if it was created */
    virtual ~MCInitialParticles() { resetBoost(); }

    /** Set the initial event values, i.e. the four momenta of both beams
     * and the vertex
     * @param her 4vector of the high energy beam
     * @param ler 4vector of the low energy beam
     * @param vertex position of the actual collision vertex
     */
    void set(const TLorentzVector& her, const TLorentzVector& ler, const TVector3& vertex)
    {
      m_her = her; m_ler = ler; m_vertex = vertex;
      resetBoost();
    }
    /** Set the High Energy Beam 4-momentum */
    void setHER(const TLorentzVector& her)
    {
      m_her = her;
      resetBoost();
    }
    /** Set the Low Energy Beam 4-momentum */
    void setLER(const TLorentzVector& ler)
    {
      m_ler = ler;
      resetBoost();
    }
    /** Set the vertex position */
    void setVertex(const TVector3& vertex)
    {
      m_vertex = vertex;
    }
    /** Set collison time */
    void setTime(double time) {m_time = time;}
    /** Get 4vector of the high energy beam */
    const TLorentzVector& getHER() const { return m_her; }
    /** Get 4vector of the low energy beam */
    const TLorentzVector& getLER() const { return m_ler; }
    /** Get the position of the collision */
    const TVector3& getVertex() const { return m_vertex; }
    /** get collison time */
    double getTime() const {return m_time;}
    /** Get the the actual collision energy (in lab system)*/
    double getEnergy() const { return (m_her + m_ler).E(); }
    /** Get the invariant mass of the collision (= energy in CMS) */
    double getMass() const { calculateBoost(); return m_invariantMass; }
    /** Return the LorentzRotation to convert from lab to CMS frame */
    const TLorentzRotation& getLabToCMS() const
    {
      calculateBoost(); return *m_labToCMS;
    }
    /** Return the LorentzRotation to convert from CMS to lab frame */
    const TLorentzRotation& getCMSToLab() const
    {
      calculateBoost(); return *m_CMSToLab;
    }

    /** Set the generation flags to be used for event generation (ORed combination of EGenerationFlags) */
    void setGenerationFlags(int flags) { m_generationFlags = flags; }
    /** Get the generation flags to be used for event generation (ORed combination of EGenerationFlags) */
    int getGenerationFlags() const { return m_generationFlags; }
    /** Check if a certain set of EGenerationFlags is set */
    bool hasGenerationFlags(int flags) const { return (m_generationFlags & flags) == flags; }
    /** Return string representation of all active flags for printing
     * @param separator separation string to be put between flags */
    std::string getGenerationFlagString(const std::string& separator = " ") const;
  private:
    /** Calculate the boost if necessary */
    void calculateBoost() const;
    /** Reset cached transformations after changing parameters. */
    void resetBoost();
    /** HER 4vector */
    TLorentzVector m_her;
    /** LER 4vector */
    TLorentzVector m_ler;
    /** collision position */
    TVector3 m_vertex;
    /** collision time */
    double m_time = 0;
    /** Boost from Lab into CMS. (calculated on first use, not saved to file) */
    mutable TLorentzRotation* m_labToCMS{nullptr}; //!transient
    /** Boost from CMS into lab. (calculated on first use, not saved to file) */
    mutable TLorentzRotation* m_CMSToLab{nullptr}; //!transient
    /** invariant mass of HER+LER (calculated on first use, not saved to file) */
    mutable double m_invariantMass{0.0}; //!transient
    /** Flags to be used when generating events */
    int m_generationFlags{0};
    /** ROOT Dictionary */
    ClassDef(MCInitialParticles, 2);
  };

  inline void MCInitialParticles::calculateBoost() const
  {
    if (m_labToCMS)
      return;

    TLorentzVector beam = m_her + m_ler;
    // Save the invariant mass because it's used very often in analysis
    m_invariantMass = beam.M();

    // If we generate events in CMS we already are in CMS and there is no
    // transformation so let's use the identity
    if (hasGenerationFlags(c_generateCMS)) {
      m_labToCMS = new TLorentzRotation();
      m_CMSToLab = new TLorentzRotation();
      return;
    }

    // Transformation from Lab system to CMS system
    m_labToCMS = new TLorentzRotation(-beam.BoostVector());
    // boost HER e- from Lab system to CMS system
    const TLorentzVector electronCMS = (*m_labToCMS) * m_her;
    // now rotate CMS such that incoming e- is parallel to z-axis
    const TVector3 zaxis(0., 0., 1.);
    TVector3 rotaxis = zaxis.Cross(electronCMS.Vect()) * (1. / electronCMS.Vect().Mag());
    double rotangle = TMath::ASin(rotaxis.Mag());
    m_labToCMS->Rotate(-rotangle, rotaxis);

    //cache derived quantities
    m_CMSToLab = new TLorentzRotation(m_labToCMS->Inverse());
  }

  inline void MCInitialParticles::resetBoost()
  {
    delete m_labToCMS;
    delete m_CMSToLab;
    m_labToCMS = nullptr;
    m_CMSToLab = nullptr;
    m_invariantMass = 0.0;
  }

} //Belle2 namespace
