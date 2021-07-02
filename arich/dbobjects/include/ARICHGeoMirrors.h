/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>
#include <TVector3.h>
#include <TRotation.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>

namespace Belle2 {

  /**
   * Geometry parameters of HAPD
   */


  class ARICHGeoMirrors: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoMirrors()
    {}

    /**
     * Consistency check of geometry parameters
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Mirror configuration parameters") const override;

    /**
     * Set parameters of mirror plate
     * @param thick thickness
     * @param width width
     * @param length length
     * @param material material name
     * @param surf optical surface
     */
    void setMirrorPlate(double thick, double width, double length, const std::string& material, const GeoOpticalSurface& surf)
    {
      m_plateThickness = thick;
      m_plateWidth = width;
      m_plateLength = length;
      m_material = material;
      m_surface = surf;
    };

    /**
     * Set number of mirror plates (slots)
     * @param nmir number of mirror plates (slots)
     */
    void setNMirrors(unsigned nmir) {m_nMirrors = nmir;}

    /**
     * Set nominal radius at which mirror plates are placed (center of plate)
     * @param r radius of mirror placement
     */
    void setRadius(double r) {m_radius = r;}

    /**
     * Set phi angle of position of the first mirror plate
     * @param stang phi angle of position of the first mirror plate
     */
    void setStartAngle(double stang) {m_startAngle = stang;}

    /**
     * Set nominal Z position of mirror plates (center point in ARICH local frame)
     * @param zPos Z position of mirror plates
     */
    void setZPosition(double zPos) {m_z = zPos;}

    /**
     * Get nominal Z position of mirror plates (center point in ARICH local frame)
     * @return Z position of mirror plates
     */
    double getZPosition() const {return m_z / s_unit;}

    /**
     * Get nominal radius at which mirror plates are placed (center of plate)
     * @return  radius of mirror placement
     */
    double getRadius() const {return m_radius / s_unit;};

    /**
     * Get phi angle of position of the first mirror plate
     * @return phi angle of position of the first mirror plate
     */
    double getStartAngle() const {return m_startAngle;};

    /**
     * Get thickness of mirror plate
     * @return mirror plate thickness
     */
    double getPlateThickness() const {return m_plateThickness / s_unit;};

    /**
     * Get width of mirror plate
     * @return mirror plate width
     */
    double getPlateWidth() const {return m_plateWidth / s_unit;};

    /**
     * Get length of mirror plate
     * @return mirror plate length
     */
    double getPlateLength() const {return m_plateLength / s_unit;};

    /**
     * Get number of mirror plates
     * @return number of mirror plates
     */
    unsigned getNMirrors() const {return m_nMirrors;}

    /**
     * Set normal vector of the i-th mirror plate surface
     * @param mirID mirror ID
     * @param x x component
     * @param y y component
     * @param z z component
     */
    void setMirrorNormVector(unsigned mirID, double x, double y, double z)
    {
      if (mirID < 1 || mirID > m_normVector.size()) B2ERROR("ARICHGeoMirrors: invalid mirror plate ID number");
      m_normVector[mirID - 1] = TVector3(x, y, z).Unit();
    }

    /**
    * Set vector to the center point of the mirror plate reflective surface
    * @param mirID mirror ID
    * @param x x component
    * @param y y component
    * @param z z component
    */
    void setMirrorPoint(unsigned mirID, double x, double y, double z)
    {
      if (mirID < 1 || mirID > m_nMirrors) B2ERROR("ARICHGeoMirrors: invalid mirror plate ID number");
      m_point[mirID - 1] = TVector3(x, y, z);
    }

    /**
     * Get material name of mirror plates
     * @return material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Returns mirror reflective optical surface
     * @return optical surface
     */
    const GeoOpticalSurface& getMirrorSurface() const {return m_surface;}

    /**
     * Get normal vector of the i-th mirror plate surface
     * @param mirID mirror ID
     * @return nomrmal vector of the mirID mirror plate
     */
    const TVector3& getNormVector(const unsigned mirID) const
    {
      if (mirID < 1 || mirID > m_nMirrors) B2ERROR("ARICHGeoMirrors: invalid mirror plate ID number");
      return m_normVector[mirID - 1];
    }

    /**
     * Get vector to the center point of the mirror plate reflective surface
     * @param mirID mirror ID
     * @return vector to the center point of mirID mirror plate
     */
    const TVector3& getPoint(const unsigned mirID) const
    {
      if (mirID < 1 || mirID > m_nMirrors) B2ERROR("ARICHGeoMirrors: invalid mirror plate ID number");
      return m_point[mirID - 1];
    }

    /**
     * Initializes default mirror position parameters (assuming plate position on the regular m_nMirrors-sided polygon,
     * with radius m_radius and with the first mirror plate positioned at m_startAngle)
     */
    void initializeDefault();

    /**
     * Set mirror plate installed/not installed (only installed plates are placed)
     * @param mirID mirror plate ID
     * @param installed true if installed
     */
    void setInstalled(unsigned mirID, bool installed)
    {
      if (mirID < 1 || mirID > m_nMirrors) B2ERROR("ARICHGeoMirrors: invalid mirror plate ID number");
      m_installed[mirID - 1] = installed;
    }

    /**
     * Check if i-th mirror plate is installed
     * @param mirID mirror plate ID
     * @return true if installed
     */
    bool isInstalled(unsigned mirID) const
    {
      if (mirID < 1 || mirID > m_nMirrors) B2ERROR("ARICHGeoMirrors: invalid mirror plate ID number");
      return m_installed[mirID - 1];
    }


  private:

    std::vector<TVector3> m_normVector;    /**< vector of normal vectors of all mirror plates */
    std::vector<TVector3> m_point;         /**< vector of points on the reflective surface of all mirror plates */
    std::vector<bool> m_installed;         /**< vector of installed/not installed flags of all mirror plates */

    std::string m_material;                /**< material of mirror plates */
    double m_z = 0;                            /**< nominal Z position of mirror plates */
    double m_radius = 0;                   /**< nominal radius at which mirror plates are placed */
    double m_startAngle = 0;               /**< phi angle of the center of the 1st mirror plate */
    double m_plateThickness = 0;           /**< thickness of mirror plate */
    double m_plateWidth = 0;               /**< width of mirror plate */
    double m_plateLength = 0;              /**< length of mirror plate */
    unsigned m_nMirrors = 0;               /**< number of mirror plates (slots) */

    GeoOpticalSurface m_surface;           /**< optical surface of mirror plates */

    ClassDefOverride(ARICHGeoMirrors, 1); /**< ClassDef */

  };

} // end namespace Belle2
