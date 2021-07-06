/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>
#include <string>
#include <vector>
#include <utility>


namespace Belle2 {

  /**
   * Geometry parameters of a quartz bar segment
   */
  class TOPGeoBarSegment: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoBarSegment()
    {}

    /**
     * Useful constructor
     * @param width bar segment width
     * @param thickness bar segment thickness
     * @param length bar segment length
     * @param material bar segment material name
     * @param name object name
     */
    TOPGeoBarSegment(double width, double thickness, double length,
                     const std::string& material,
                     const std::string& name = "TOPBarSegment"): TOPGeoBase(name),
      m_width(width), m_thickness(thickness), m_length(length), m_material(material)
    {}

    /**
     * Sets glue parameters (for glue on -z side)
     * @param thickness glue thickness
     * @param material glue material name
     */
    virtual void setGlue(double thickness, const std::string& material)
    {
      m_glueThickness = thickness;
      m_glueMaterial = material;
    }

    /**
     * Sets glue to be broken (delaminated)
     * @param fraction fraction of the delaminated surface
     * @param angle angle of the delaminated surface
     * @param material material name to simulate the delaminated glue
     */
    virtual void setGlueDelamination(double fraction, double angle,
                                     const std::string& material)
    {
      m_brokenFraction = fraction;
      m_brokenAngle = angle;
      m_brokenGlueMaterial = material;
    }

    /**
     * Sets optical surface
     * @param surface optical surface
     * @param sigmaAlpha geant4 parameter for surface roughness
     */
    void setSurface(const GeoOpticalSurface& surface, double sigmaAlpha)
    {
      m_surface = surface;
      m_sigmaAlpha = sigmaAlpha;
    }

    /**
     * Sets vendor's name and serial number
     * @param vendor vendor's name
     * @param serialNumber serial number
     */
    void setVendorData(const std::string& vendor, const std::string& serialNumber)
    {
      m_vendor = vendor;
      m_serialNumber = serialNumber;
    }

    /**
     * Returns bar segment width
     * @return width
     */
    double getWidth() const {return m_width / s_unit;}

    /**
     * Returns bar segment thickness
     * @return thickness
     */
    double getThickness() const {return m_thickness / s_unit;}

    /**
     * Returns bar segment length
     * @return length
     */
    double getLength() const {return m_length / s_unit;}

    /**
     * Returns bar segment length including glue
     * @return full length
     */
    double getFullLength() const {return (m_length + m_glueThickness) / s_unit;}

    /**
     * Returns bar segment material name
     * @return material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Returns glue thickness (glue on -z side)
     * @return thickness
     */
    virtual double getGlueThickness() const {return m_glueThickness / s_unit;}

    /**
     * Returns glue material name (glue on -z side)
     * @return material name
     */
    const std::string& getGlueMaterial() const {return m_glueMaterial;}

    /**
     * Returns fraction of the delaminated surface
     * @return fraction
     */
    double getBrokenGlueFraction() const {return m_brokenFraction;}

    /**
     * Returns angle of the delaminated surface
     * @return angle
     */
    double getBrokenGlueAngle() const {return m_brokenAngle;}

    /**
     * Returns material name which represents broken glue
     * @return material name
     */
    const std::string& getBrokenGlueMaterial() const {return m_brokenGlueMaterial;}

    /**
     * Returns the x-y contour of broken glue
     */
    std::vector<std::pair<double, double> > getBrokenGlueContour() const;

    /**
     * Returns optical surface
     * @return optical surface
     */
    const GeoOpticalSurface& getSurface() const {return m_surface;}

    /**
     * Returns surface reflectivity at given photon energy
     * @param energy photon energy in [eV]
     * @return reflectivity
     */
    double getSurfaceReflectivity(double energy) const
    {
      return TOPGeoBase::getReflectivity(m_surface, energy);
    }

    /**
     * Returns geant4 parameter describing surface roughness
     * @return surface roughness
     */
    double getSigmaAlpha() const {return m_sigmaAlpha;}

    /**
     * Returns vendor's name
     * @return name
     */
    const std::string& getVendor() const {return m_vendor;}

    /**
     * Returns serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    virtual bool isConsistent() const override;


    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "Bar segment geometry parameters") const override;

  protected:

    /**
     * Construct a 2D contour
     * @param A dimension in x
     * @param B dimension in y
     * @param fraction surface fraction
     * @param angle angle
     * @param contour clock-wise polygon to return
     */
    void constructContour(double A, double B, double fraction, double angle,
                          std::vector<std::pair<double, double> >& contour) const;

    float m_width = 0; /**< bar segment width */
    float m_thickness = 0; /**< bar segment thickness */
    float m_length = 0; /**< bar segment length */
    std::string m_material; /**< bar segment material name */
    float m_glueThickness = 0; /**< glue thickness */
    std::string m_glueMaterial; /**< glue material name */
    GeoOpticalSurface m_surface; /**< optical surface */
    float m_sigmaAlpha = 0; /**< geant4 parameter for surface roughness */
    float m_brokenFraction = 0; /**< fraction of broken (delaminated) glue */
    float m_brokenAngle = 0; /**< angle of broken (delaminated) glue */
    std::string m_brokenGlueMaterial; /**< broken glue material name */
    std::string m_vendor; /**< vendor's name */
    std::string m_serialNumber; /**< serial number */

    ClassDefOverride(TOPGeoBarSegment, 3); /**< ClassDef */

  };

} // end namespace Belle2
