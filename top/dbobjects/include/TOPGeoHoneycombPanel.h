/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <string>
#include <vector>
#include <utility>

namespace Belle2 {

  /**
   * Geometry parameters of honeycomb panel
   */
  class TOPGeoHoneycombPanel: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoHoneycombPanel()
    {}

    /**
     * Full constructor
     * @param width width
     * @param length length
     * @param minThickness minimal thickness
     * @param maxThickness maximal thickness
     * @param radius radius of curved surface
     * @param edgeWidth width of the reinforced edge
     * @param y position of the flat surface in local (bar) frame
     * @param N number of flat surfaces for approximation of curved surface
     * @param material material name
     * @param edgeMaterial material name of reinforced edge
     * @param name volume name in Geant
     */
    TOPGeoHoneycombPanel(double width,
                         double length,
                         double minThickness,
                         double maxThickness,
                         double radius,
                         double edgeWidth,
                         double y,
                         int N,
                         const std::string& material,
                         const std::string& edgeMaterial,
                         const std::string& name = "TOPHoneycombPanel"): TOPGeoBase(name),
      m_width(width), m_length(length), m_minThickness(minThickness),
      m_maxThickness(maxThickness), m_radius(radius), m_edgeWidth(edgeWidth),
      m_y(y), m_N(N), m_material(material), m_edgeMaterial(edgeMaterial)
    {}

    /**
     * Returns panel width
     * @return width
     */
    double getWidth() const {return m_width / s_unit;}

    /**
     * Returns panel length
     * @return length
     */
    double getLength() const {return m_length / s_unit;}

    /**
     * Returns panel minimal thickness
     * @return thickness
     */
    double getMinThickness() const {return m_minThickness / s_unit;}

    /**
     * Returns panel maximal thickness
     * @return thickness
     */
    double getMaxThickness() const {return m_maxThickness / s_unit;}

    /**
     * Returns radius of curved surface
     * @return radius
     */
    double getRadius() const {return m_radius / s_unit;}

    /**
     * Returns width of the reinforced edge
     * @return width
     */
    double getEdgeWidth() const {return m_edgeWidth / s_unit;}

    /**
     * Returns y position of the flat surface in local (bar) frame
     * @return y
     */
    double getY() const {return m_y / s_unit;}

    /**
     * Returns material name
     * @return material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Returns material name of reinforced edge
     * @return material name
     */
    const std::string& getEdgeMaterial() const {return m_edgeMaterial;}


    /**
     * Appends curved surface to contour
     * @param contour 2D contour
     * @param y0 offset in y0
     * @param fromLeftToRight if true, going from left to right otherwise oposite
     */
    void appendContour(std::vector<std::pair<double, double> >& contour,
                       double y0,
                       bool fromLeftToRight) const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "QBB honeycomb panel geometry parameters") const override;

  private:

    float m_width = 0;  /**< width */
    float m_length = 0; /**< length */
    float m_minThickness = 0; /**< minimal thickness */
    float m_maxThickness = 0; /**< maximal thickness */
    float m_radius = 0; /**< radius of curved surface */
    float m_edgeWidth = 0; /**< width of the reinforced edge */
    float m_y = 0; /**< y position of the flat surface in local (bar) frame */
    int m_N = 0; /**< number of flat surfaces to approximate curved surface */
    std::string m_material; /**< material name */
    std::string m_edgeMaterial; /**< material name of reinforced edge */

    ClassDefOverride(TOPGeoHoneycombPanel, 1); /**< ClassDef */

  };


} // end namespace Belle2

