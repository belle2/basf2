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
#include <math.h>
#include <TVector2.h>

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
     * Unfolded prism exit window.
     */
    struct UnfoldedWindow {
      double y0 = 0; /**< window origin in y */
      double z0 = 0; /**< window origin in z */
      double sy = 0; /**< window surface direction in y */
      double sz = 0; /**< window surface direction in z */
      double ny = 0; /**< normal to window surface, y component */
      double nz = 0; /**< normal to window surface, z component */
      double nsy[2] = {0, 0}; /**< normals to upper [0] and slanted [1] surfaces, y component */
      double nsz[2] = {0, 0}; /**< normals to upper [0] and slanted [1] surfaces, z component */

      /**
       * constructor
       * @param orig window origin
       * @param dir window surface direction (= upper surface normal)
       * @param norm window surface normal (pointing out of prism)
       * @param slanted slanted surface normal
       */
      UnfoldedWindow(const TVector2& orig, const TVector2& dir, const TVector2& norm, const TVector2& slanted);
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
     * @param flatLength length of the flat part at the bottom
     * @param material prism material name
     * @param name object name
     */
    TOPGeoPrism(double width, double thickness, double length,
                double exitThickness, double flatLength,
                const std::string& material,
                const std::string& name = "TOPPrism_"):
      TOPGeoBarSegment(width, thickness, length, material, name),
      m_exitThickness(exitThickness), m_flatLength(flatLength)
    {}

    /**
     * Recalculates flatLength according to given prism angle
     * @param angle prism angle
     */
    void setAngle(double angle)
    {
      m_flatLength = m_length - (m_exitThickness - m_thickness) / tan(angle);
    }

    /**
     * Sets parameters of the peel-off cookie volumes
     * @param size volume size in x
     * @param offset offset in x of the center of volume ID = 1
     * @param thickness volume thickness
     * @param material material name
     */
    void setPeelOffRegions(double size, double offset, double thickness,
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
     * Returns prism angle
     * @return angle
     */
    double getAngle() const
    {
      return atan((m_exitThickness - m_thickness) / (m_length - m_flatLength));
    }

    /**
     * Returns wavelength filter thickness (filter on -z side).
     * For backward compatibility, since now the filter is part of TOPGeoPMTArray
     * @return thickness
     */
    double getFilterThickness() const {return m_glueThickness / s_unit;}

    /**
     * Returns wavelength filter material name (filter on -z side)
     * For backward compatibility, since now the filter is part of TOPGeoPMTArray
     * @return material name
     */
    const std::string& getFilterMaterial() const {return m_glueMaterial;}

    /**
     * Returns glue thickness (no glue on -z side)
     * @return thickness
     */
    double getGlueThickness() const override {return 0;}

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
     * Returns unfolded exit windows.
     * Exit window position is defined where the slanted surface is cutted off.
     * @return vector of unfolded exit windows
     */
    const std::vector<UnfoldedWindow>& getUnfoldedWindows() const
    {
      if (m_unfoldedWindows.empty()) unfold();
      return m_unfoldedWindows;
    }

    /**
     * Returns index of true window in a vector of unfolded exit windows.
     * @return index
     */
    int getK0() const
    {
      if (m_unfoldedWindows.empty()) unfold();
      return m_k0;
    }

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Prism geometry parameters") const override;


  private:

    /**
     * Disable setting the glue, since there is none
     */
    void setGlue(double, const std::string&) override {}

    /**
     * Disable setting the glue delamination, since there is no glue
     */
    void setGlueDelamination(double, double, const std::string&) override {}

    /**
     * Unfold prism exit window
     */
    void unfold() const;

    /**
     * Do unfolding
     * @param points points defining position of upper and slanted surfaces
     * @param normals normals of upper and slanted surfaces
     * @param orig origin of exit window
     * @param surf exit window surface direction
     * @param norm exit window surface normal
     * @param slanted slanted surface normal
     * @param k index of the surface to start unfolding (0 or 1)
     * @param result the result of unfolding
     */
    void reflect(const TVector2* points, const TVector2* normals,
                 const TVector2& orig, const TVector2& surf, const TVector2& norm, const TVector2& slanted, int k,
                 std::vector<UnfoldedWindow>& result) const;

    float m_exitThickness = 0; /**< thickness at PMT side */
    float m_flatLength = 0; /**< length of the flat part at the bottom */

    float m_peelOffSize = 0;       /**< size in x of peel-off volume */
    float m_peelOffOffset = 0;     /**< offset in x of the peel-off volume ID = 1 */
    float m_peelOffThickness = 0;  /**< thickness of peel-off volume */
    std::string m_peelOffMaterial; /**< material name of peel-off volume */
    std::vector<PeelOffRegion> m_peelOffRegions; /**< peel-off regions */

    /** cache for unfolded prism exit windows */
    mutable std::vector<UnfoldedWindow> m_unfoldedWindows; //! do not write out
    /** cache for the index of true window in unfolded prism exit windows */
    mutable int m_k0 = 0; //! do not write out

    ClassDefOverride(TOPGeoPrism, 2); /**< ClassDef */

  };

} // end namespace Belle2
