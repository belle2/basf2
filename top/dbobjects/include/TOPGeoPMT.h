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

namespace Belle2 {

  /**
   * Geometry parameters of MCP-PMT
   */
  class TOPGeoPMT: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoPMT()
    {}

    /**
     * Useful constructor
     * @param sizeX full size in x
     * @param sizeY full size in y
     * @param sizeZ full size in z
     * @param name object name
     */
    TOPGeoPMT(double sizeX, double sizeY, double sizeZ,
              const std::string& name = "TOPPMT"):
      TOPGeoBase(name), m_sizeX(sizeX), m_sizeY(sizeY), m_sizeZ(sizeZ)
    {}

    /**
     * Sets wall thickness
     * @param thickness wall thickness
     */
    void setWallThickness(double thickness) {m_wallThickness = thickness;}

    /**
     * Sets casing material
     * @param material casing material name
     */
    void setWallMaterial(const std::string& material) {m_wallMaterial = material;}

    /**
     * Sets inside material
     * @param material inside material name
     */
    void setFillMaterial(const std::string& material) {m_fillMaterial = material;}

    /**
     * Sets sensitive volume (photo-cathode)
     * @param sizeX size in x
     * @param sizeY size in y
     * @param thickness thickness
     * @param material sensitive material
     */
    void setSensVolume(double sizeX, double sizeY, double thickness,
                       const std::string& material)
    {
      m_sensSizeX = sizeX;
      m_sensSizeY = sizeY;
      m_sensThickness = thickness;
      m_sensMaterial = material;
    }

    /**
     * Sets number of pixel rows and columns
     * @param numColumns number of pixel columns
     * @param numRows number of pixel rows
     */
    void setNumPixels(unsigned numColumns, unsigned numRows)
    {
      m_numColumns = numColumns;
      m_numRows = numRows;
    }

    /**
     * Sets entrance window
     * @param thickness thickness
     * @param material window material
     */
    void setWindow(double thickness, const std::string& material)
    {
      m_winThickness = thickness;
      m_winMaterial = material;
    }

    /**
     * Sets bottom
     * @param thickness thickness
     * @param material bottom material
     */
    void setBottom(double thickness, const std::string& material)
    {
      m_botThickness = thickness;
      m_botMaterial = material;
    }

    /**
     * Sets reflective edge
     * @param width width
     * @param thickness thickness
     * @param surf optical surface
     */
    void setReflEdge(double width, double thickness, const GeoOpticalSurface& surf)
    {
      m_reflEdgeWidth = width;
      m_reflEdgeThickness = thickness;
      m_reflEdgeSurface = surf;
    }

    /**
     * Returns full size in x
     * @return size in x
     */
    double getSizeX() const {return m_sizeX / s_unit;}

    /**
     * Returns full size in y
     * @return size in y
     */
    double getSizeY() const {return m_sizeY / s_unit;}

    /**
     * Returns full size in z
     * @return size in z
     */
    double getSizeZ() const {return m_sizeZ / s_unit;}

    /**
     * Returns wall thickness
     * @return wall thickness
     */
    double getWallThickness() const {return m_wallThickness / s_unit;}

    /**
     * Returns wall (casing) material name
     * @return material name
     */
    const std::string& getWallMaterial() const {return m_wallMaterial;}

    /**
     * Returns fill (inside) material name
     * @return material name
     */
    const std::string& getFillMaterial() const {return m_fillMaterial;}

    /**
     * Returns sensitive volume (photo-cathode) size in x
     * @return size in x
     */
    double getSensSizeX() const {return m_sensSizeX / s_unit;}

    /**
     * Returns sensitive volume (photo-cathode) size in y
     * @return size in y
     */
    double getSensSizeY() const {return m_sensSizeY / s_unit;}

    /**
     * Returns sensitive volume (photo-cathode) thickness
     * @return thickness
     */
    double getSensThickness() const {return m_sensThickness / s_unit;}

    /**
     * Returns number of pixel columns
     * @return number of pixel columns
     */
    unsigned getNumColumns() const {return m_numColumns;}

    /**
     * Returns number of pixel rows
     * @return number of pixel rows
     */
    unsigned getNumRows() const {return m_numRows;}

    /**
     * Returns number of pixels
     * @return number of pixels
     */
    unsigned getNumPixels() const {return m_numColumns * m_numRows;}

    /**
     * Returns sensitive material name
     * @return material name
     */
    const std::string& getSensMaterial() const {return m_sensMaterial;}

    /**
     * Returns entrance window thickness
     * @return thickness
     */
    double getWinThickness() const {return m_winThickness / s_unit;}

    /**
     * Returns entrance window material name
     * @return material name
     */
    const std::string& getWinMaterial() const {return m_winMaterial;}

    /**
     * Returns bottom thickness
     * @return thickness
     */
    double getBotThickness() const {return m_botThickness / s_unit;}

    /**
     * Returns bottom material name
     * @return material name
     */
    const std::string& getBotMaterial() const {return m_botMaterial;}

    /**
     * Returns reflective edge width
     * @return width
     */
    double getReflEdgeWidth() const {return m_reflEdgeWidth / s_unit;}

    /**
     * Returns reflective edge thickness
     * @return thickness
     */
    double getReflEdgeThickness() const {return m_reflEdgeThickness / s_unit;}

    /**
     * Returns reflective edge optical surface
     * @return optical surface
     */
    const GeoOpticalSurface& getReflEdgeSurface() const {return m_reflEdgeSurface;}

    /**
     * Converts x-coordinate to pixel column (1-based)
     * @param x coordinate
     * @return pixel column or 0 if x outside sensitive area
     */
    unsigned getPixelColumn(double x) const;

    /**
     * Converts y-coordinate to pixel row (1-based)
     * @param y coordinate
     * @return pixel row or 0 if y outside sensitive area
     */
    unsigned getPixelRow(double y) const;

    /**
     * Converts x and y coordinates to PMT pixel ID (1-based)
     * @param x coordinate
     * @param y coordinate
     * @return pixel ID or 0 if x or y outside sensitive area
     */
    unsigned getPixelID(double x, double y) const;

    /**
     * Returns x coordinate of pixel center
     * @param col valid column number (1-based)
     * @return x coordinate of pixel center
     */
    double getX(unsigned col) const;

    /**
     * Returns y coordinate of pixel center
     * @param row valid row number (1-based)
     * @return y coordinate of pixel center
     */
    double getY(unsigned row) const;

    /**
     * Returns pixel size in x
     * @return pixel size in x
     */
    double getDx() const {return getSensSizeX() / m_numColumns;}

    /**
     * Returns pixel size in y
     * @return pixel size in y
     */
    double getDy() const {return getSensSizeY() / m_numRows;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "MCP-PMT geometry parameters") const override;

  private:

    float m_sizeX = 0; /**< full size in x */
    float m_sizeY = 0; /**< full size in y */
    float m_sizeZ = 0; /**< full size in z */
    float m_wallThickness = 0;  /**< side wall thickness */
    std::string m_wallMaterial; /**< casing material */
    std::string m_fillMaterial; /**< inside material */

    float m_sensSizeX = 0; /**< sensitive volume (photo-cathode) size in x */
    float m_sensSizeY = 0; /**< sensitive volume (photo-cathode) size in y */
    float m_sensThickness = 0; /**< sensitive volume (photo-cathode) thickness */
    unsigned m_numColumns = 0; /**< number of pixel columns */
    unsigned m_numRows = 0;    /**< number of pixel rows */
    std::string m_sensMaterial; /**< sensitive material */

    float m_winThickness = 0; /**< window thickness */
    std::string m_winMaterial; /**< window material */

    float m_botThickness = 0; /**< bottom thickness */
    std::string m_botMaterial; /**< bottom material */

    float m_reflEdgeWidth = 0; /**< reflective edge width */
    float m_reflEdgeThickness = 0; /**< reflective edge thickness */
    GeoOpticalSurface m_reflEdgeSurface; /**< reflective edge optical surface */

    ClassDefOverride(TOPGeoPMT, 1); /**< ClassDef */

  };

} // end namespace Belle2
