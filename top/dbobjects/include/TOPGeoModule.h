/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <top/dbobjects/TOPGeoBarSegment.h>
#include <top/dbobjects/TOPGeoMirrorSegment.h>
#include <top/dbobjects/TOPGeoPrism.h>
#include <top/dbobjects/TOPGeoPMTArray.h>
#include <top/dbobjects/TOPGeoPMTArrayDisplacement.h>
#include <top/dbobjects/TOPGeoModuleDisplacement.h>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {

  /**
   * Geometry parameters of a module (optical components + positioning)
   */
  class TOPGeoModule: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoModule()
    {}

    /**
     * Useful constructor
     * @param moduleID module ID
     * @param radius radius of the bar central plane in Belle II frame
     * @param phi azimuthal angle in Belle II frame
     * @param backwardZ z position of prism-bar joint in Belle II frame
     * @param name object name
     */
    TOPGeoModule(int moduleID, double radius, double phi, double backwardZ,
                 const std::string& name = "TOPModule"): TOPGeoBase(name),
      m_moduleID(moduleID), m_radius(radius), m_phi(phi), m_backwardZ(backwardZ)
    {}

    /**
     * Copy constructor
     */
    TOPGeoModule(const TOPGeoModule& module);

    /**
     * Assignment operator
     */
    TOPGeoModule& operator=(const TOPGeoModule& module);

    /**
     * Destructor
     */
    ~TOPGeoModule();

    /**
     * Sets module construction number (0 = ideal module = default)
     * @param moduleCNumber construction number
     */
    void setModuleCNumber(unsigned moduleCNumber) {m_moduleCNumber = moduleCNumber;}

    /**
     * Sets bar segment No.1 (forward bar)
     * @param bar segment No.1 geometry parameters
     */
    void setBarSegment1(const TOPGeoBarSegment& bar) {m_bar1 = bar;}

    /**
     * Sets bar segment No.2 (backward bar)
     * @param bar segment No.2 geometry parameters
     */
    void setBarSegment2(const TOPGeoBarSegment& bar) {m_bar2 = bar;}

    /**
     * Sets mirror segment
     * @param mirror mirror segment geometry parameters
     */
    void setMirrorSegment(const TOPGeoMirrorSegment& mirror) {m_mirror = mirror;}

    /**
     * Sets prism
     * @param prism prism geometry parameters
     */
    void setPrism(const TOPGeoPrism& prism) {m_prism = prism;}

    /**
     * Sets PMT array
     * @param array PMT array geometry parameters
     */
    void setPMTArray(const TOPGeoPMTArray& array) {m_pmtArray = array;}

    /**
     * Sets PMT as optically decoupled
     * @param pmtID ID of PMT to be set as decoupled (1-based)
     */
    void setDecoupledPMT(unsigned pmtID) {m_pmtArray.setDecoupledPMT(pmtID);}

    /**
     * Generate randomly a fraction of PMT's to be optically decoupled
     * @param fraction decoupled fraction
     */
    void generateDecoupledPMTs(double fraction)
    {
      m_pmtArray.generateDecoupledPMTs(fraction);
    }

    /**
     * Sets glue to be broken (delaminated)
     * @param glueID glue ID: bar-mirror (1), bar-bar (2), prism-bar (3)
     * @param fraction fraction of the delaminated surface
     * @param angle angle of the delaminated surface
     * @param material material name to simulate the delaminated glue
     */
    void setBrokenGlue(int glueID, double fraction, double angle,
                       const std::string& material);

    /**
     * Sets parameters of the peel-off cookie volumes
     * @param thickness volume thickness
     * @param material material name
     */
    void setPeelOffRegions(double thickness, const std::string& material);

    /**
     * Appends peel-off cookie region
     * @param ID region ID (1-based)
     * @param fraction fraction of the area
     * @param angle angle of the area
     */
    void appendPeelOffRegion(unsigned ID, double fraction, double angle)
    {
      m_prism.appendPeelOffRegion(ID, fraction, angle);
    }


    /**
     * Sets PMT array displacement
     * @param displ PMT array displacement parameters
     */
    void setPMTArrayDisplacement(const TOPGeoPMTArrayDisplacement& displ)
    {
      m_arrayDisplacement = displ;
    }

    /**
     * Sets module displacement
     * @param displ module displacement parameters
     */
    void setModuleDisplacement(const TOPGeoModuleDisplacement& displ)
    {
      m_moduleDisplacement = displ;
    }

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const {return m_moduleID;}

    /**
     * Returns radius of the bar central plane in Belle II frame
     * @return radius
     */
    double getRadius() const {return m_radius / s_unit;}

    /**
     * Returns module azimuthal angle in Belle II frame
     * @return azimuthal angle
     */
    double getPhi() const {return m_phi;}

    /**
     * Returns the z position of prism-bar joint in Belle II frame
     * @return z position
     */
    double getBackwardZ() const {return m_backwardZ / s_unit;}

    /**
     * Returns module construction number (0 = ideal module)
     * @return construction number
     */
    unsigned getModuleCNumber() const {return m_moduleCNumber;}

    /**
     * Returns bar segment No.1 (forward bar)
     * @return bar segment geometry parameters
     */
    const TOPGeoBarSegment& getBarSegment1() const {return m_bar1;}

    /**
     * Returns bar segment No.2 (backward bar)
     * @return bar segment geometry parameters
     */
    const TOPGeoBarSegment& getBarSegment2() const {return m_bar2;}

    /**
     * Returns mirror segment
     * @return mirror segment geometry parameters
     */
    const TOPGeoMirrorSegment& getMirrorSegment() const {return m_mirror;}

    /**
     * Returns prism
     * @return prism geometry parameters
     */
    const TOPGeoPrism& getPrism() const {return m_prism;}

    /**
     * Returns PMT array
     * @return PMT array geometry parameters
     */
    const TOPGeoPMTArray& getPMTArray() const {return m_pmtArray;}

    /**
     * Returns PMT array displacement
     * @return PMT array displacement parameters
     */
    const TOPGeoPMTArrayDisplacement& getPMTArrayDisplacement() const
    {
      return m_arrayDisplacement;
    }

    /**
     * Returns module displacement
     * @return module displacement parameters
     */
    const TOPGeoModuleDisplacement& getModuleDisplacement() const
    {
      return m_moduleDisplacement;
    }

    /**
     * Returns bar length (w/o prism) including glue joints
     * @return length
     */
    double getBarLength() const
    {
      return m_bar1.getFullLength() + m_bar2.getFullLength() + m_mirror.getFullLength();
    }

    /**
     * Returns full length (including prism, wavelenght filter and glue joints)
     * @return length
     */
    double getFullLength() const {return getBarLength() + m_prism.getFullLength();}

    /**
     * Returns the z position of the bar forward end in Belle II frame
     * @return z position
     */
    double getForwardZ() const {return getBackwardZ() + getBarLength();}

    /**
     * Returns average bar thickness (weigthed by segment lengths)
     * @return average bar thickness
     */
    double getBarThickness() const
    {
      return (m_bar1.getThickness() * m_bar1.getFullLength() +
              m_bar2.getThickness() * m_bar2.getFullLength() +
              m_mirror.getThickness() * m_mirror.getFullLength()) / getBarLength();
    }

    /**
     * Returns average bar width (weigthed by segment lengths)
     * @return average bar width
     */
    double getBarWidth() const
    {
      return (m_bar1.getWidth() * m_bar1.getFullLength() +
              m_bar2.getWidth() * m_bar2.getFullLength() +
              m_mirror.getWidth() * m_mirror.getFullLength()) / getBarLength();
    }

    /**
     * Returns average surface reflectivity at given photon energy (weigthed by segment lengths)
     * @param energy photon energy in [eV]
     * @return average reflectivity
     */
    double getSurfaceReflectivity(double energy) const
    {
      return (m_bar1.getSurfaceReflectivity(energy) * m_bar1.getFullLength() +
              m_bar2.getSurfaceReflectivity(energy) * m_bar2.getFullLength() +
              m_mirror.getSurfaceReflectivity(energy) * m_mirror.getFullLength()) / getBarLength();
    }

    /**
     * Returns z of bar center (w/o prism) in Belle II frame
     * @return z coordinate if bar center
     */
    double getZc() const {return (getBackwardZ() + getForwardZ()) / 2;}

    /**
     * Returns inner surface radius in Belle II frame
     * @return radius
     */
    double getInnerRadius() const {return getRadius() - getBarThickness() / 2;}

    /**
     * Transforms 3D point from Belle II to module internal (= nominal & displaced) frame
     * @param point 3D point in Belle II frame (basf2 units!)
     * @return 3D point in module internal frame (basf2 units!)
     */
    TVector3 pointToLocal(const TVector3& point) const;

    /**
     * Transforms momentum vector from Belle II to module internal (= nominal & displaced) frame
     * @param momentum momentum vector in Belle II frame
     * @return momentum vector in module internal frame
     */
    TVector3 momentumToLocal(const TVector3& momentum) const;

    /**
     * Transforms 3D point from module internal (= nominal & displaced) frame to Belle II frame
     * @param point 3D point in module internal frame (basf2 units!)
     * @return 3D point in Belle II frame (basf2 units!)
     */
    TVector3 pointToGlobal(const TVector3& point) const;

    /**
     * Transforms momentum vector from module internal (= nominal & displaced) frame to Belle II frame
     * @param momentum momentum vector in module internal frame
     * @return momentum vector in Belle II frame
     */
    TVector3 momentumToGlobal(const TVector3& momentum) const;

    /**
     * Transforms 3D point from Belle II to module nominal frame
     * @param point 3D point in Belle II frame (basf2 units!)
     * @return 3D point in module nominal frame (basf2 units!)
     */
    TVector3 pointGlobalToNominal(const TVector3& point) const;

    /**
     * Transforms momentum vector from Belle II to module nominal frame
     * @param momentum momentum vector in Belle II frame
     * @return momentum vector in module nominal frame
     */
    TVector3 momentumGlobalToNominal(const TVector3& momentum) const;

    /**
     * Transforms 3D point from module nominal frame to Belle II frame
     * @param point 3D point in module nominal frame (basf2 units!)
     * @return 3D point in Belle II frame (basf2 units!)
     */
    TVector3 pointNominalToGlobal(const TVector3& point) const;

    /**
     * Transforms momentum vector from module nominal frame to Belle II frame
     * @param momentum momentum vector in module nominal frame
     * @return momentum vector in Belle II frame
     */
    TVector3 momentumNominalToGlobal(const TVector3& momentum) const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Module geometry parameters") const override;


  private:

    /**
     * Sets transformation cache
     */
    void setTransformation() const;

    int m_moduleID = 0; /**< module ID */
    float m_radius = 0; /**< radius of bar central plane in Belle II frame */
    float m_phi = 0; /**< azimuthal angle in Belle II frame */
    float m_backwardZ = 0; /**< z position of prism-bar joint in Belle II frame */

    unsigned m_moduleCNumber = 0; /**< module construction number, 0 = ideal module */
    TOPGeoBarSegment m_bar1; /**< bar segment 1 (forward bar) */
    TOPGeoBarSegment m_bar2; /**< bar segment 2 (backward bar) */
    TOPGeoMirrorSegment m_mirror; /**< mirror segment */
    TOPGeoPrism m_prism; /**< prism */
    TOPGeoPMTArray m_pmtArray;  /**< geometry parameters of PMT array */
    TOPGeoPMTArrayDisplacement m_arrayDisplacement;  /**< PMT array displacement */
    TOPGeoModuleDisplacement m_moduleDisplacement;   /**< module displacement */

    /** cache for rotation matrix from internal (= nominal & displaced) to Belle II frame */
    mutable TRotation* m_rotation = 0;    //! do not write out
    /** cache for inverse rotation matrix */
    mutable TRotation* m_rotationInverse = 0;    //! do not write out
    /** cache for translation vector from internal (= nominal & displaced) to Belle II frame */
    mutable TVector3* m_translation = 0;  //! do not write out

    /** cache for rotation matrix from nominal to Belle II frame */
    mutable TRotation* m_rotationNominal = 0;    //! do not write out
    /** cache for inverse rotation matrix */
    mutable TRotation* m_rotationNominalInverse = 0;    //! do not write out
    /** cache for translation vector from nominal to Belle II frame */
    mutable TVector3* m_translationNominal = 0;  //! do not write out

    ClassDefOverride(TOPGeoModule, 3); /**< ClassDef */

  };

} // end namespace Belle2
