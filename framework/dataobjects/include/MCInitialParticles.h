/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Const.h>
#include <framework/utilities/LabToCms.h>

#include <TVector3.h>
#include <Math/Boost.h>
#include <Math/LorentzRotation.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

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
      m_vertex(b.m_vertex), m_validFlag(b.m_validFlag), m_generationFlags(b.m_generationFlags) {}

    /** Assignment operator */
    MCInitialParticles& operator=(const MCInitialParticles& b)
    {
      m_her = b.m_her; m_ler = b.m_ler; m_vertex = b.m_vertex;
      m_validFlag = b.m_validFlag; m_generationFlags = b.m_generationFlags;
      resetBoost();
      return *this;
    }

    /** Equality operator */
    bool operator==(const MCInitialParticles& b) const
    {
      // FIXME: sin(x) returns slightly different values on
      // different platforms in some cases so we cannot just do an equality
      // comparison. We need to do this more elegantly, this is just for
      // testing if it solves all problems
#if defined(MCP_DBL_CMP) || defined(MCP_VEC3_CMP) || defined(MCP_VEC4_CMP)
#error Macro already defined, cannot continue
#endif
#define MCP_DBL_CMP(a,b,x) ((a.X()==b.X())||(std::abs(a.X()-b.X())<1e-10))
#define MCP_VEC3_CMP(a,b) (MCP_DBL_CMP(a,b,X) && MCP_DBL_CMP(a,b,Y) && MCP_DBL_CMP(a,b,Z))
#define MCP_VEC4_CMP(a,b) (MCP_VEC3_CMP(a,b) && MCP_DBL_CMP(a,b,E))
      return MCP_VEC4_CMP(m_her, b.m_her) && MCP_VEC4_CMP(m_ler, b.m_ler) && MCP_VEC3_CMP(m_vertex, b.m_vertex)
             && m_validFlag == b.m_validFlag &&
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
    void set(const ROOT::Math::PxPyPzEVector& her, const ROOT::Math::PxPyPzEVector& ler, const ROOT::Math::XYZVector& vertex)
    {
      m_her = her;
      m_ler = ler;
      m_vertex = vertex;
      m_validFlag = true;
      resetBoost();
    }

    /** Initialize the event values from CMS energy and parameters of the Lorentz transformation between LAB and CMS.
     *  In addition the vertex is also initialized.
     * @param Ecms     centre-of-mass energy of the collision
     * @param bX       x-component of the boost vector, i.e. of (pHER + pLER) / (eHER + eLER), where pHER & pLER are momentum 3-vectors
     * @param bY       y-component of the boost vector, i.e. of (pHER + pLER) / (eHER + eLER), where pHER & pLER are momentum 3-vectors
     * @param bZ       z-component of the boost vector, i.e. of (pHER + pLER) / (eHER + eLER), where pHER & pLER are momentum 3-vectors
     * @param angleXZ  angle in the XZ plane of the collision axis in the CM system obtained by pure boost
     * @param angleYZ  angle in the YZ plane of the collision axis in the CM system obtained by pure boost
     * @param vertex   position of the actual collision vertex
     */
    void setByLorentzTransformation(double Ecms, double bX, double bY, double bZ, double angleXZ, double angleYZ,
                                    const ROOT::Math::XYZVector& vertex)
    {
      if (m_labToCMS) delete m_labToCMS;
      if (m_CMSToLab) delete m_CMSToLab;

      m_invariantMass = Ecms;
      m_CMSToLab      = new ROOT::Math::LorentzRotation();
      m_labToCMS      = new ROOT::Math::LorentzRotation();
      *m_CMSToLab     = cmsToLab(bX, bY, bZ, angleXZ, angleYZ);
      *m_labToCMS     = m_CMSToLab->Inverse();

      const double me = Const::electron.getMass();
      double p = sqrt(Ecms * Ecms / 4 - me * me);
      m_her = (*m_CMSToLab) * ROOT::Math::PxPyPzEVector(0.0, 0.0,  p, Ecms / 2);
      m_ler = (*m_CMSToLab) * ROOT::Math::PxPyPzEVector(0.0, 0.0, -p, Ecms / 2);

      m_vertex = vertex;
      m_validFlag = true;
    }

    /** Set the High Energy Beam 4-momentum */
    void setHER(const ROOT::Math::PxPyPzEVector& her)
    {
      m_her = her;
      resetBoost();
    }

    /** Set the Low Energy Beam 4-momentum */
    void setLER(const ROOT::Math::PxPyPzEVector& ler)
    {
      m_ler = ler;
      resetBoost();
    }

    /** Set the vertex position */
    void setVertex(const ROOT::Math::XYZVector& vertex)
    {
      m_vertex = vertex;
    }

    /** Set collision time */
    void setTime(double time) {m_time = time;}

    /** Set the generation flags to be used for event generation (ORed combination of EGenerationFlags) */
    virtual void setGenerationFlags(int flags) { m_generationFlags = flags; }

    /** Get 4vector of the high energy beam */
    const ROOT::Math::PxPyPzEVector& getHER() const { return m_her; }

    /** Get 4vector of the low energy beam */
    const ROOT::Math::PxPyPzEVector& getLER() const { return m_ler; }

    /** Get the position of the collision */
    const ROOT::Math::XYZVector& getVertex() const { return m_vertex; }

    /** Get collision time */
    double getTime() const {return m_time;}

    /** Get the actual collision energy (in lab system)*/
    double getEnergy() const { return (m_her + m_ler).E(); }

    /** Get the invariant mass of the collision (= energy in CMS) */
    double getMass() const { calculateBoost(); return m_invariantMass; }

    /** Get the boost vector (velocity of system produced in the collision) */
    ROOT::Math::XYZVector getBoostVector() const { calculateBoost(); return -(m_her + m_ler).BoostToCM(); }

    /** Get the 4-vector of electron beam in CM system obtained by pure boost */
    ROOT::Math::PxPyPzEVector getBoostedHER() const { calculateBoost(); return m_boostedHER; }

    /** Return the LorentzRotation to convert from lab to CMS frame */
    const ROOT::Math::LorentzRotation& getLabToCMS() const
    {
      calculateBoost(); return *m_labToCMS;
    }

    /** Return the LorentzRotation to convert from CMS to lab frame */
    const ROOT::Math::LorentzRotation& getCMSToLab() const
    {
      calculateBoost(); return *m_CMSToLab;
    }

    /** Get the flag to check if a valid MCInitialParticles object was already generated and filled in an event. */
    bool getValidFlag() const { return m_validFlag; }

    /** Get the generation flags to be used for event generation (ORed combination of EGenerationFlags) */
    int getGenerationFlags() const { return m_generationFlags; }

    /** Check if a certain set of EGenerationFlags is set */
    bool hasGenerationFlags(int flags) const { return (m_generationFlags & flags) == flags; }

    /** Return string representation of all active flags for printing
     * @param separator separation string to be put between flags */
    std::string getGenerationFlagString(const std::string& separator = " ") const;


    /** Return the LorentzRotation from CMS to LAB based on the following parameters
     * @param bX       x-component of the boost vector, i.e. of (pHER + pLER) / (eHER + eLER), where pHER & pLER are momentum 3-vectors
     * @param bY       y-component of the boost vector, i.e. of (pHER + pLER) / (eHER + eLER), where pHER & pLER are momentum 3-vectors
     * @param bZ       z-component of the boost vector, i.e. of (pHER + pLER) / (eHER + eLER), where pHER & pLER are momentum 3-vectors
     * @param angleXZ  angle in the XZ plane of the collision axis in the CM system obtained by pure boost
     * @param angleYZ  angle in the YZ plane of the collision axis in the CM system obtained by pure boost
     */
    static ROOT::Math::LorentzRotation cmsToLab(double bX, double bY, double bZ, double angleXZ, double angleYZ);

  private:

    /** Calculate the Lorentz transformations LAB->CMS & CMS->LAB if necessary */
    void calculateBoost() const;
    /** Reset cached transformations after changing parameters. */
    void resetBoost();
    /** HER 4vector */
    ROOT::Math::PxPyPzEVector m_her;
    /** LER 4vector */
    ROOT::Math::PxPyPzEVector m_ler;
    /** collision position */
    ROOT::Math::XYZVector m_vertex;
    /** collision time */
    double m_time = 0;
    /** Boost from Lab into CMS. (calculated on first use, not saved to file) */
    mutable ROOT::Math::LorentzRotation* m_labToCMS{nullptr}; //!transient
    /** Boost from CMS into lab. (calculated on first use, not saved to file) */
    mutable ROOT::Math::LorentzRotation* m_CMSToLab{nullptr}; //!transient
    /** invariant mass of HER+LER (calculated on first use, not saved to file) */
    mutable double m_invariantMass{0.0}; //!transient
    /** HER 4-momentum in CM frame obtained by pure boost (calculated on first use, not saved to file) */
    mutable ROOT::Math::PxPyPzEVector m_boostedHER; //!transient
    /** Flag to check if a valid MCInitialParticles object was already generated and filled in an event. */
    bool m_validFlag = false;

  protected:

    /** Flags to be used when generating events */
    int m_generationFlags{0};

  private:

    /** ROOT Dictionary */
    ClassDef(MCInitialParticles, 5);
  };

  inline void MCInitialParticles::calculateBoost() const
  {
    if (m_labToCMS)
      return;

    ROOT::Math::PxPyPzEVector beam = m_her + m_ler;
    // Save the invariant mass because it's used very often in analysis
    m_invariantMass = beam.M();

    // If we generate events in CMS we already are in CMS and there is no
    // transformation so let's use the identity
    if (hasGenerationFlags(c_generateCMS)) {
      m_labToCMS = new ROOT::Math::LorentzRotation();
      m_CMSToLab = new ROOT::Math::LorentzRotation();
      return;
    }

    m_boostedHER = ROOT::Math::LorentzRotation(ROOT::Math::Boost(beam.BoostToCM())) * m_her;
    double cmsAngleXZ = atan(m_boostedHER.X() / m_boostedHER.Z());
    double cmsAngleYZ = atan(m_boostedHER.Y() / m_boostedHER.Z());
    m_labToCMS = new ROOT::Math::LorentzRotation(
      LabToCms::rotateLabToCms(-beam.BoostToCM(), cmsAngleXZ, cmsAngleYZ)
    );

    //cache derived quantities
    m_CMSToLab = new ROOT::Math::LorentzRotation(m_labToCMS->Inverse());
  }





  inline void MCInitialParticles::resetBoost()
  {
    delete m_labToCMS;
    delete m_CMSToLab;
    m_labToCMS = nullptr;
    m_CMSToLab = nullptr;
    m_invariantMass = 0.0;
    m_boostedHER = ROOT::Math::PxPyPzEVector();
  }

} //Belle2 namespace
