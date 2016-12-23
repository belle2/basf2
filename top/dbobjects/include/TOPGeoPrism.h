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

#include <top/dbobjects/TOPGeoBarSegment.h>
#include <string>
#include <vector>
#include <utility>

namespace Belle2 {

  /**
   * Geometry parameters of prism
   */
  class TOPGeoPrism: public TOPGeoBarSegment {
  public:

    /**
     * Parameters of peel-off cookie region (corresponds to 2x2 PMT's)
     */
    struct PeelOffRegion {
      unsigned ID; /**< ID of the region (1-based) */
      float fraction; /**< fraction of peel-off area */
      float angle;    /**< angle of peel-off area */
    };

    /**
     * Default constructor
     */
    TOPGeoPrism()
    {}

    /**
     * Useful constructor
     * @param width prism width
     * @param thickness thickness at bar side
     * @param length prism length
     * @param exitThickness thickness at PMT side
     * @param flatLenght length of the flat part at the bottom
     * @param material prism material name
     * @param name object name
     */
    TOPGeoPrism(double width, double thickness, double length,
                double exitThickness, double flatLength,
                const std::string& material,
                const std::string& name = "TOPPrism"):
      TOPGeoBarSegment(width, thickness, length, material, name),
      m_exitThickness(exitThickness), m_flatLength(flatLength)
    {}

    /**
     * Sets parameters of the peel-off cookie volumes
     * @param size volume size in x
     * @param offset offset in x of the center of volume ID = 1
     * @param thickness volume thickness
     * @param material material name
     */
    void setPeelOffData(double size, double offset, double thickness,
                        const std::string& material)
    {
      m_peelOffSize = size;
      m_peelOffOffset = offset;
      m_peelOffThickness = thickness;
      m_peelOffMaterial = material;
    }

    /**
     * Appends peel-off cookie region
     * @param ID region ID (1-based)
     * @param fraction fraction of the area
     * @param angle angle of the area
     */
    void appendPeelOffRegion(unsigned ID, double fraction, double angle);

    /**
     * Returns prism thickness at PMT side
     * @return thickness at PMT side
     */
    double getExitThickness() const {return m_exitThickness / s_unit;}

    /**
     * Returns the length of a flat surface at prism bottom
     * @return bottom flat surface length
     */
    double getFlatLength() const {return m_flatLength / s_unit;}

    /**
     * Returns the length of a flat surface at prism bottom including filter
     * @return bottom flat surface full length
     */
    double getFullFlatLength() const {return (m_flatLength + m_glueThickness) / s_unit;}

    /**
     * Returns wavelength filter thickness (filter on -z side)
     * @return thickness
     */
    double getFilterThickness() const {return m_glueThickness / s_unit;}

    /**
     * Returns wavelength filter material name (filter on -z side)
     * @return material name
     */
    const std::string& getFilterMaterial() const {return m_glueMaterial;}

    /**
     * Returns glue thickness (no glue on -z side, instead there is a wavelength filter)
     * @return thickness
     */
    double getGlueThickness() const {return 0;}

    /**
     * Returns cookie size in x (corresponding to 2x2 PMT)
     * @return size in x
     */
    double getPeelOffSize() const {return m_peelOffSize / s_unit;}

    /**
     * Returns peel-off thickness
     * @return thickness
     */
    double getPeelOffThickness() const {return m_peelOffThickness / s_unit;}

    /**
     * Returns peel-off material
     * @return material name
     */
    const std::string& getPeelOffMaterial() const {return m_peelOffMaterial;}

    /**
     * Returns peel-off cookie regions
     * @return vector of regions
     */
    const std::vector<PeelOffRegion>& getPeelOffRegions() const {return m_peelOffRegions;}

    /**
     * Returns peel-off offset in x of the given region
     * @param region peel-off region
     * @return offset in x
     */
    double getPeelOffCenter(const PeelOffRegion& region) const
    {
      return (m_peelOffOffset - (region.ID - 1) * m_peelOffSize) / s_unit;
    }

    /**
     * Returns the x-y contour of the peel-off region
     * @param region peel-off region
     * @return clock-wise polygon
     */
    std::vector<std::pair<double, double> > getPeelOffContour(const PeelOffRegion& region) const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Prism geometry parameters") const;


  private:

    float m_exitThickness = 0; /**< thickness at PMT side */
    float m_flatLength = 0; /**< length of the flat part at the bottom */

    float m_peelOffSize = 0;       /**< size in x of peel-off volume */
    float m_peelOffOffset = 0;     /**< offset in x of the peel-off volume ID = 1 */
    float m_peelOffThickness = 0;  /**< thickness of peel-off volume */
    std::string m_peelOffMaterial; /**< material name of peel-off volume */
    std::vector<PeelOffRegion> m_peelOffRegions; /**< peel-off regions */

    ClassDef(TOPGeoPrism, 2); /**< ClassDef */

  };

} // end namespace Belle2
