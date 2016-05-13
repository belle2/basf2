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
#include <top/dbobjects/TOPGeoPMT.h>

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
     */
    TOPGeoPMTArray(unsigned numColumns, unsigned numRows,
                   double gapX, double gapY,
                   const std::string& material,
                   const TOPGeoPMT& pmt):
      m_numRows(numRows), m_numColumns(numColumns), m_dx(gapX), m_dy(gapY),
      m_material(material), m_pmt(pmt)
    {
      m_dx += pmt.getSizeX() * s_unit;
      m_dy += pmt.getSizeY() * s_unit;
      m_sizeX = numColumns * m_dx - gapX;
      m_sizeY = numRows * m_dy - gapY;
    }

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
     * Returns array volume dimension in x
     * @return size in x
     */
    double getSizeX() const {return m_sizeX / s_unit;}

    /**
     * Returns array volume dimension in y
     * @return size in y
     */
    double getSizeY() const {return m_sizeY / s_unit;}

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
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "PMT array geometry parameters") const;

  private:

    unsigned m_numRows = 0;     /**< number of rows */
    unsigned m_numColumns = 0;  /**< number of columns */
    float m_dx = 0; /**< spacing in x */
    float m_dy = 0; /**< spacing in y */
    float m_sizeX = 0; /**< size in x */
    float m_sizeY = 0; /**< size in y */
    std::string m_material; /**< material name into which PMT's are inserted */
    TOPGeoPMT m_pmt; /**< PMT geometry parameters */

    ClassDef(TOPGeoPMTArray, 1); /**< ClassDef */

  };

} // end namespace Belle2
