/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexandre BEAULIEU                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>

namespace Belle2 {

  /**
  * The Class for COIL geometry parameters
  */

  class COILGeometryPar: public TObject {

  public:

    //! Default constructor
    COILGeometryPar();

    //! Destructor
    ~COILGeometryPar();

    //
    // Setters and Getters
    //
    ///////////////////////

    //! Get COIL geometry version
    int getVersion() const { return m_Version; }

    //! Set COIL geometry version
    void setVersion(int version) { m_Version = version; }


    //! Get Global Rotation Angle
    double getGlobalRotAngle() const
    {
      return m_GlobalRotAngle;
    }

    //! Set Global Rotation Angle
    void setGlobalRotAngle(double angle)
    {
      m_GlobalRotAngle = angle;
    }

    //! Get Global offset in Z
    double getGlobalOffsetZ() const
    {
      return m_GlobalOffsetZ;
    }

    //! Set Global offset in Z
    void setGlobalOffsetZ(double offset)
    {
      m_GlobalOffsetZ = offset;
    }


    //
    // Cryostat vessel
    //

    //! Get the Material of the cryostat vessel
    std::string getCryoMaterial() const { return m_strMatCryo; }

    //! Set the Material of the cryostat vessel (as a std::string)
    void setCryoMaterial(const std::string& material) { m_strMatCryo = material; }

    //! Get the Inner radius of the cryostat vessel
    double getCryoRmin() const
    {
      return m_CryoRmin;
    }

    //! Set the Inner radius of the cryostat vessel
    void setCryoRmin(double r)
    {
      m_CryoRmin = r;
    }

    //! Get the Outer radius of the cryostat vessel
    double getCryoRmax() const
    {
      return m_CryoRmax;
    }

    //! Set the Outer radius of the cryostat vessel
    void setCryoRmax(double r)
    {
      m_CryoRmax = r;
    }

    //! Get the Half-length of the cryostat vessel
    double getCryoLength() const
    {
      return m_CryoLength;
    }

    //! Set the Half-length of the cryostat vessel
    void setCryoLength(double l)
    {
      m_CryoLength = l;
    }


    //
    // cavity #1
    //

    //! Get the Material of the cavity #1
    std::string getCav1Material() const { return m_strMatCav1; }

    //! Set the Material of the cavity #1 (as a std::string)
    void setCav1Material(const std::string& material) { m_strMatCav1 = material; }

    //! Get the Inner radius of the cavity #1
    double getCav1Rmin() const
    {
      return m_Cav1Rmin;
    }

    //! Set the Inner radius of the cavity #1
    void setCav1Rmin(double r)
    {
      m_Cav1Rmin = r;
    }

    //! Get the Outer radius of the cavity #1
    double getCav1Rmax() const
    {
      return m_Cav1Rmax;
    }

    //! Set the Outer radius of the cavity #1
    void setCav1Rmax(double r)
    {
      m_Cav1Rmax = r;
    }

    //! Get the Half-length of the cavity #1
    double getCav1Length() const
    {
      return m_Cav1Length;
    }

    //! Set the Half-length of the cavity #1
    void setCav1Length(double l)
    {
      m_Cav1Length = l;
    }


    //
    // cavity #2
    //

    //! Get the Material of the cavity #2
    std::string getCav2Material() const { return m_strMatCav2; }

    //! Set the Material of the cavity #2 (as a std::string)
    void setCav2Material(const std::string& material) { m_strMatCav2 = material; }

    //! Get the Inner radius of the cavity #2
    double getCav2Rmin() const
    {
      return m_Cav2Rmin;
    }

    //! Set the Inner radius of the cavity #2
    void setCav2Rmin(double r)
    {
      m_Cav2Rmin = r;
    }

    //! Get the Outer radius of the cavity #2
    double getCav2Rmax() const
    {
      return m_Cav2Rmax;
    }

    //! Set the Outer radius of the cavity #2
    void setCav2Rmax(double r)
    {
      m_Cav2Rmax = r;
    }

    //! Get the Half-length of the cavity #2
    double getCav2Length() const
    {
      return m_Cav2Length;
    }

    //! Set the Half-length of the cavity #2
    void setCav2Length(double l)
    {
      m_Cav2Length = l;
    }


    //
    // radiation shield
    //

    //! Get the Material of the radiation shield
    std::string getShieldMaterial() const { return m_strMatShield; }

    //! Set the Material of the radiation shield (as a std::string)
    void setShieldMaterial(const std::string& material) { m_strMatShield = material; }

    //! Get the Inner radius of the radiation shield
    double getShieldRmin() const
    {
      return m_ShieldRmin;
    }

    //! Set the Inner radius of the radiation shield
    void setShieldRmin(double r)
    {
      m_ShieldRmin = r;
    }

    //! Get the Outer radius of the radiation shield
    double getShieldRmax() const
    {
      return m_ShieldRmax;
    }

    //! Set the Outer radius of the radiation shield
    void setShieldRmax(double r)
    {
      m_ShieldRmax = r;
    }

    //! Get the Half-length of the radiation shield
    double getShieldLength() const
    {
      return m_ShieldLength;
    }

    //! Set the Half-length of the radiation shield
    void setShieldLength(double l)
    {
      m_ShieldLength = l;
    }


    //
    // superconducting coil
    //

    //! Get the Material of the superconducting coil
    std::string getCoilMaterial() const { return m_strMatCoil; }

    //! Set the Material of the superconducting coil (as a std::string)
    void setCoilMaterial(const std::string& material) { m_strMatCoil = material; }

    //! Get the Inner radius of the superconducting coil
    double getCoilRmin() const
    {
      return m_CoilRmin;
    }

    //! Set the Inner radius of the superconducting coil
    void setCoilRmin(double r)
    {
      m_CoilRmin = r;
    }

    //! Get the Outer radius of the superconducting coil
    double getCoilRmax() const
    {
      return m_CoilRmax;
    }

    //! Set the Outer radius of the superconducting coil
    void setCoilRmax(double r)
    {
      m_CoilRmax = r;
    }

    //! Get the Half-length of the superconducting coil
    double getCoilLength() const
    {
      return m_CoilLength;
    }

    //! Set the Half-length of the superconducting coil
    void setCoilLength(double l)
    {
      m_CoilLength = l;
    }



  private:


    //
    //         Data members:
    // these are the geometry parameters
    //
    ////////////////////////////////////

    //! Geometry version
    int m_Version;

    //
    // Global parameters
    //
    //! Global rotation angle bewteen coil and detector
    double m_GlobalRotAngle;
    //! Global offset along beam axis between coil and detector
    double m_GlobalOffsetZ;

    //
    // Cryostat vessel
    //
    //! Material of the cryostat vessel
    std::string m_strMatCryo;
    //! Inner radius of the cryostat vessel
    double m_CryoRmin;
    //! Outer radius of the cryostat vessel
    double m_CryoRmax;
    //! Half-length  of the cryostat vessel
    double m_CryoLength;

    //
    // Cavity #1
    //
    //! Material of the cavity #1
    std::string m_strMatCav1;
    //! Inner radius of the cavity #1
    double m_Cav1Rmin;
    //! Inner radius of the cavity #1
    double m_Cav1Rmax;
    //! Inner radius of the cavity #1
    double m_Cav1Length;

    //
    // Cavity #2
    //
    //! Material of the cavity #2
    std::string m_strMatCav2;
    //! Inner radius of the cavity #2
    double m_Cav2Rmin;
    //! Inner radius of the cavity #2
    double m_Cav2Rmax;
    //! Inner radius of the cavity #2
    double m_Cav2Length;

    //
    // Radiation Shield
    //
    //! Material of the Radiation Shield
    std::string m_strMatShield;
    //! Inner radius of the Radiation Shield
    double m_ShieldRmin;
    //! Inner radius of the Radiation Shield
    double m_ShieldRmax;
    //! Inner radius of the Radiation Shield
    double m_ShieldLength;

    //
    // Superconducting Coil
    //
    //! Material of the Superconducting Coil
    std::string m_strMatCoil;
    //! Inner radius of the Superconducting Coil
    double m_CoilRmin;
    //! Inner radius of the Superconducting Coil
    double m_CoilRmax;
    //! Inner radius of the Superconducting Coil
    double m_CoilLength;

    //
    // For ROOT objects
    //

    ClassDef(COILGeometryPar, 1);  /**< ClassDef, must be the last term before the closing {}*/


  };
} // end of namespace Belle2


