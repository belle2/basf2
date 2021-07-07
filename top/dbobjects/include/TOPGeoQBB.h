/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <top/dbobjects/TOPGeoHoneycombPanel.h>
#include <top/dbobjects/TOPGeoSideRails.h>
#include <top/dbobjects/TOPGeoEndPlate.h>
#include <top/dbobjects/TOPGeoPrismEnclosure.h>
#include <top/dbobjects/TOPGeoColdPlate.h>
#include <string>
#include <vector>
#include <utility>

namespace Belle2 {

  /**
   * Geometry parameters of Quartz Bar Box  (mother class)
   */
  class TOPGeoQBB: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoQBB()
    {}

    /**
     * Useful constructor
     * @param width full width
     * @param length full length
     * @param prismPosition position of prism-bar joint wrt QBB back
     * @param material material inside QBB
     * @param name volume name in Geant
     */
    TOPGeoQBB(double width, double length, double prismPosition,
              const std::string& material,
              const std::string& name = "TOPQBB"): TOPGeoBase(name),
      m_width(width), m_length(length), m_prismPosition(prismPosition), m_material(material)
    {}

    /**
     * Sets inner honeycomb panel
     * @param innerPanel geometry parameters
     */
    void setInnerPanel(const TOPGeoHoneycombPanel& innerPanel)
    {
      m_innerPanel = innerPanel;
    }

    /**
     * Sets outer honeycomb panel
     * @param outerPanel geometry parameters
     */
    void setOuterPanel(const TOPGeoHoneycombPanel& outerPanel)
    {
      m_outerPanel = outerPanel;
    }

    /**
     * Sets side rails
     * @param sideRails geometry parameters
     */
    void setSideRails(const TOPGeoSideRails& sideRails) {m_sideRails = sideRails;}

    /**
     * Sets prism enclosure
     * @param prismEnclosure geometry parameters
     */
    void setPrismEnclosure(const TOPGeoPrismEnclosure& prismEnclosure)
    {
      m_prismEnclosure = prismEnclosure;
    }

    /**
     * Sets forward end plate
     * @param endPlate geometry parameters
     */
    void setEndPlate(const TOPGeoEndPlate& endPlate) {m_endPlate = endPlate;}

    /**
     * Sets forward cold plate
     * @param coldPlate geometry parameters
     */
    void setColdPlate(const TOPGeoColdPlate& coldPlate) {m_coldPlate = coldPlate;}

    /**
     * Returns full width
     * @return width
     */
    double getWidth() const {return m_width / s_unit;}

    /**
     * Returns panel width used in x-y contours of honeycomb panels
     * @return width
     */
    double getPanelWidth() const
    {
      return getWidth() - 2 * m_sideRails.getThickness();
    }

    /**
     * Returns full length
     * @return length
     */
    double getLength() const {return m_length / s_unit;}

    /**
     * Returns side rails length
     * @return length
     */
    double getSideRailsLength() const
    {
      return getLength() - m_prismEnclosure.getBackThickness() - m_endPlate.getThickness();
    }

    /**
     * Returns the position of prism-bar joint wrt QBB back
     * @return position
     */
    double getPrismPosition() const {return m_prismPosition / s_unit;}

    /**
     * Returns the name of material inside QBB
     * @return material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Returns inner honeycomb panel
     * @return panel geometry parameters
     */
    const TOPGeoHoneycombPanel& getInnerPanel() const {return m_innerPanel;}

    /**
     * Returns outer honeycomb panel
     * @return panel geometry parameters
     */
    const TOPGeoHoneycombPanel& getOuterPanel() const {return m_outerPanel;}

    /**
     * Returns side rails
     * @return side rails geometry parameters
     */
    const TOPGeoSideRails& getSideRails() const {return m_sideRails;}

    /**
     * Returns prism enclosure
     * @return prism enclosure geometry parameters
     */
    const TOPGeoPrismEnclosure& getPrismEnclosure() const {return m_prismEnclosure;}

    /**
     * Returns forward end plate
     * @return end plate geometry parameters
     */
    const TOPGeoEndPlate& getForwardEndPlate() const {return m_endPlate;}

    /**
     * Returns cold plate
     * @return cold plate geometry parameters
     */
    const TOPGeoColdPlate& getColdPlate() const {return m_coldPlate;}

    /**
     * Returns forward x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getForwardContour() const;

    /**
     * Returns inner honeycomb panel x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getInnerPanelContour() const;

    /**
     * Returns outer honeycomb panel x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getOuterPanelContour() const;

    /**
     * Returns backward x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getBackwardContour() const;

    /**
     * Returns prism enclosure wall x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getPrismEnclosureContour() const;

    /**
     * Returns prism enclosure back plate x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getBackPlateContour() const;

    /**
     * Returns prism enclosure front plate x-y contour
     * @return polygon
     */
    std::vector<std::pair<double, double> > getFrontPlateContour() const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "QBB geometry parameters") const override;

    /**
     * Print forward contour (polygon) - for debugging
     */
    void printForwardContour() const;

    /**
     * Print inner honeycomb panel contour (polygon) - for debugging
     */
    void printInnerPanelContour() const;

    /**
     * Print outer honeycomb panel contour (polygon) - for debugging
     */
    void printOuterPanelContour() const;

    /**
     * Print backward contour (polygon) - for debugging
     */
    void printBackwardContour() const;

    /**
     * Print prism enclosure contour (polygon) - for debugging
     */
    void printPrismEnclosureContour() const;

    /**
     * Print prism enclosure back plate contour (polygon) - for debugging
     */
    void printBackPlateContour() const;

    /**
     * Print prism enclosure front plate contour (polygon) - for debugging
     */
    void printFrontPlateContour() const;

  private:

    float m_width = 0;  /**< full width */
    float m_length = 0; /**< full length */
    float m_prismPosition = 0; /**< position of prism-bar joint wrt QBB back */
    std::string m_material; /**< material inside QBB */
    TOPGeoHoneycombPanel m_innerPanel; /**< inner honeycomb panel */
    TOPGeoHoneycombPanel m_outerPanel; /**< outer honeycomb panel */
    TOPGeoSideRails m_sideRails; /**< side rails */
    TOPGeoPrismEnclosure m_prismEnclosure; /**< prism enclosure */
    TOPGeoEndPlate m_endPlate; /**< forward end plate */
    TOPGeoColdPlate m_coldPlate; /**< cold plate */

    ClassDefOverride(TOPGeoQBB, 2); /**< ClassDef */

  };

} // end namespace Belle2
