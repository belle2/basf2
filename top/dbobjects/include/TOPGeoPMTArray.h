/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <top/dbobjects/TOPGeoPMT.h>
#include <vector>

namespace Belle2 {

  /**
   * Geometry parameters of MCP-PMT array
   */
  class TOPGeoPMTArray: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoPMTArray()
    {}

    /**
     * Full constructor
     * @param numColumns number of columns
     * @param numRows number of rows
     * @param gapX gap between PMT's in x
     * @param gapY gap between PMT's in y
     * @param material material name into which PMT's are inserted
     * @param pmt PMT geometry parameters
     * @param name object name
     */
    TOPGeoPMTArray(unsigned numColumns, unsigned numRows,
                   double gapX, double gapY,
                   const std::string& material,
                   const TOPGeoPMT& pmt,
                   const std::string& name = "TOPPMTArray"):
      TOPGeoBase(name),
      m_numRows(numRows), m_numColumns(numColumns), m_dx(gapX), m_dy(gapY),
      m_gx(gapX), m_gy(gapY), m_material(material), m_pmt(pmt)
    {
      m_dx += pmt.getSizeX() * s_unit;
      m_dy += pmt.getSizeY() * s_unit;
    }

    /**
     * Sets air gap for optically decoupled PMT's
     * @param gap gap thickness
     */
    void setAirGap(double gap) {m_airGap = gap;}

    /**
     * Sets PMT as optically decoupled
     * @param pmtID ID of PMT to be set as decoupled (1-based)
     */
    void setDecoupledPMT(unsigned pmtID) {m_decoupledPMTs.push_back(pmtID);}

    /**
     * Sets silicone cookie
     * @param thickness thickness
     * @param material material name
     */
    void setSiliconeCookie(double thickness, const std::string& material)
    {
      m_cookieThickness = thickness;
      m_cookieMaterial = material;
    }

    /**
     * Sets wavelength filter
     * @param thickness thickness
     * @param material material name
     */
    void setWavelengthFilter(double thickness, const std::string& material)
    {
      m_filterThickness = thickness;
      m_filterMaterial = material;
    }

    /**
     * Generate randomly a fraction of PMT's to be optically decoupled
     * @param fraction decoupled fraction
     */
    void generateDecoupledPMTs(double fraction);

    /**
     * Returns number of array columns
     * @return number of columns
     */
    unsigned getNumColumns() const {return m_numColumns;}

    /**
     * Returns number of array rows
     * @return number of rows
     */
    unsigned getNumRows() const {return m_numRows;}

    /**
     * Returns array size
     * @return array size
     */
    unsigned getSize() const {return m_numRows * m_numColumns;}

    /**
     * Returns number of pixels
     * @return number of pixels
     */
    unsigned getNumPixels() const {return getSize() * m_pmt.getNumPixels();}

    /**
     * Returns number of pixel columns
     * @return number of pixel columns
     */
    unsigned getNumPixelColumns() const {return m_numColumns * m_pmt.getNumColumns();}

    /**
     * Returns number of pixel rows
     * @return number of pixel rows
     */
    unsigned getNumPixelRows() const {return m_numRows * m_pmt.getNumRows();}

    /**
     * Returns spacing in x (column width)
     * @return spacing in x
     */
    double getDx() const {return m_dx / s_unit;}

    /**
     * Returns spacing in y (row height)
     * @return spacing in y
     */
    double getDy() const {return m_dy / s_unit;}

    /**
     * Returns gap between PMT's in x
     * @return gap in x
     */
    double getGapX() const {return m_gx / s_unit;}

    /**
     * Returns gap between PMT's in y
     * @return gap in y
     */
    double getGapY() const {return m_gy / s_unit;}

    /**
     * Returns silicone cookie thickness
     * @return thickness
     */
    double getCookieThickness() const {return m_cookieThickness / s_unit;}

    /**
     * Returns silicone cookie material
     * @return material name
     */
    const std::string& getCookieMaterial() const {return m_cookieMaterial;}

    /**
     * Returns wavelength filter thickness
     * @return thickness
     */
    double getFilterThickness() const {return m_filterThickness / s_unit;}

    /**
     * Returns wavelenght filter material
     * @return material name
     */
    const std::string& getFilterMaterial() const {return m_filterMaterial;}

    /**
     * Returns array volume dimension in x
     * @return size in x
     */
    double getSizeX() const {return m_numColumns * getDx();}

    /**
     * Returns array volume dimension in y
     * @return size in y
     */
    double getSizeY() const {return m_numRows * getDy();}

    /**
     * Returns array volume dimension in z
     * @return size in z
     */
    double getSizeZ() const
    {
      return m_pmt.getSizeZ() + getAirGap() + getCookieThickness() + getFilterThickness();
    }

    /**
     * Returns x coordinate of column center
     * @param col valid column number (1-based)
     * @return x coordinate of column center
     */
    double getX(unsigned col) const
    {
      return (int)(m_numColumns + 1 - 2 * col) / 2.0 * getDx();
    }

    /**
     * Returns the y coordinate of row center
     * @param row valid row number (1-based)
     * @return y coordinate of row center
     */
    double getY(unsigned row) const
    {
      return (int)(2 * row - m_numRows - 1) / 2.0 * getDy();
    }

    /**
     * Returns material name into which PMT's are inserted
     * @return material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Returns PMT geometry parameters
     * @return PMT geometry parameters
     */
    const TOPGeoPMT& getPMT() const {return m_pmt;}

    /**
     * Converts row and column numbers to PMT ID (1-based)
     * @param row row number (1-based)
     * @param col column number (1-based)
     * @return PMT ID or 0 for invalid row or column
     */
    unsigned getPmtID(unsigned row, unsigned col) const;

    /**
     * Converts PMT ID to row number (1-based)
     * @param pmtID PMT ID (1-based)
     * @return row number or 0 for invalid PMT ID
     */
    unsigned getRow(unsigned pmtID) const;

    /**
     * Converts PMT ID to column number (1-based)
     * @param pmtID PMT ID (1-based)
     * @return column number or 0 for invalid PMT ID
     */
    unsigned getColumn(unsigned pmtID) const;

    /**
     * Converts (digitizes) x, y and PMT ID to pixel ID (1-based)
     * @param x coordinate in PMT local frame
     * @param y coordinate in PMT local frame
     * @param pmtID PMT ID (1-based)
     * @return pixel ID or 0 if (x,y) outside sensitive area or pmtID invalid
     */
    int getPixelID(double x, double y, unsigned pmtID) const;

    /**
     * Returns pixel ID (1-based)
     * @param pmtID PMT ID (1-based)
     * @param pmtPixelID PMT pixel ID (1-based)
     * @return pixel ID or 0 for invalid input ID's
     */
    int getPixelID(unsigned pmtID, unsigned pmtPixelID) const;

    /**
     * Returns air gap
     * @return gap thickness
     */
    double getAirGap() const {return m_airGap / s_unit;}

    /**
     * Returns ID's of optically decoupled PMT's
     * @return PMT ID (1-based)
     */
    const std::vector<unsigned>& getDecoupledPMTs() const {return m_decoupledPMTs;}

    /**
     * Checks if PMT is optically decoupled
     * @return true if decoupled
     */
    bool isPMTDecoupled(unsigned pmtID) const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "PMT array geometry parameters") const override;

  private:

    unsigned m_numRows = 0;     /**< number of rows */
    unsigned m_numColumns = 0;  /**< number of columns */
    float m_dx = 0; /**< spacing in x */
    float m_dy = 0; /**< spacing in y */
    float m_gx = 0; /**< gap between PMT's in x */
    float m_gy = 0; /**< gap between PMT's  in y */
    std::string m_material; /**< material name into which PMT's are inserted */
    TOPGeoPMT m_pmt; /**< PMT geometry parameters */
    float m_airGap = 0; /**< air gap thickness for decoupled PMT's */
    std::vector<unsigned> m_decoupledPMTs; /**< ID's of decoupled PMT's */
    float m_cookieThickness = 0; /**< silicone cookie thickness */
    std::string m_cookieMaterial; /**< silicone cookie material */
    float m_filterThickness = 0; /**< wavelength filter thickness */
    std::string m_filterMaterial; /**< wavelength filter material */

    ClassDefOverride(TOPGeoPMTArray, 3); /**< ClassDef */

  };

} // end namespace Belle2
