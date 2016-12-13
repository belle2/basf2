/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>
#include <string>

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
    void setGlue(double thickness, const std::string& material)
    {
      m_glueThickness = thickness;
      m_glueMaterial = material;
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
     * Returns optical surface
     * @return optical surface
     */
    const GeoOpticalSurface& getSurface() const {return m_surface;}

    /**
     * Returns geant4 parameter describing surface roughness
     * @return surface roughness
     */
    double getSigmaAlpha() const {return m_sigmaAlpha;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    virtual bool isConsistent() const;


    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "Bar segment geometry parameters") const;

  protected:

    float m_width = 0; /**< bar segment width */
    float m_thickness = 0; /**< bar segment thickness */
    float m_length = 0; /**< bar segment length */
    std::string m_material; /**< bar segment material name */
    float m_glueThickness = 0; /**< glue thickness */
    std::string m_glueMaterial; /**< glue material name */
    GeoOpticalSurface m_surface; /**< optical surface */
    float m_sigmaAlpha = 0; /**< geant4 parameter for surface roughness */

    ClassDef(TOPGeoBarSegment, 1); /**< ClassDef */

  };

} // end namespace Belle2
