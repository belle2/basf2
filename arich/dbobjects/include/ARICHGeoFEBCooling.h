/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <arich/dbobjects/tessellatedSolidStr.h>
#include <string>

//root
#include <TVector3.h>

namespace Belle2 {

  /**
   * Geometry parameters of Cooling System - version2 (v2).
   * Upgrade of the cooling system have been done after phase 2 (in september - november 2018).
   * This data base object contains information about :
   * FEB cooling bodies. https://kds.kek.jp/indico/event/28886/session/2/contribution/7/material/slides/0.pdf
   * Merger cooling bodies.
   * Cooling pipes.
   */
  class ARICHGeoFEBCooling: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoFEBCooling()
    {}

    /**
     * Set size of the small square in mm
     * @param value size of small the square in mm
     */
    void setSmallSquareSize(double value) {m_smallSquareSize = value;}

    /**
     * Set thickness of the small square in mm
     * @param value thickness of small the square in mm
     */
    void setSmallSquareThickness(double value) {m_smallSquareThickness = value;}

    /**
     * Set size of the big square in mm
     * @param value size of big the square in mm
     */
    void setBigSquareSize(double value) {m_bigSquareSize = value;}

    /**
     * Set thickness of the big square in mm
     * @param value thickness of big the square in mm
     */
    void setBigSquareThickness(double value) {m_bigSquareThickness = value;}

    /**
     * Set length of the rectangle in mm
     * @param value length of the rectangle in mm
     */
    void setRectangleL(double value) { m_rectangleL = value; }

    /**
     * Set width of the rectangle in mm
     * @param value width of the rectangle in mm
     */
    void setRectangleW(double value) { m_rectangleW = value; }

    /**
     * Set thickness of the rectangle in mm
     * @param value thickness of the rectangle in mm
     */
    void setRectangleThickness(double value) { m_rectangleThickness = value; }

    /**
     * Set distance from center of the rectangle in mm
     * @param value distance from center of the rectangle in mm
     */
    void setRectangleDistanceFromCenter(double value) { m_rectangleDistanceFromCenter = value; }

    /**
     * Set vector of feb cooling configuration/geometry ID.
     * 0 -> Not installed
     * 1 -> Default position
     * 2 -> Rotated by 90 degree
     * @param febcoolingv2GeometryID vector of feb cooling configuration/geometry ID.
     */
    void setFebcoolingv2GeometryID(const std::vector<double>& febcoolingv2GeometryID) { m_febcoolingv2GeometryID = febcoolingv2GeometryID; }

    /**
     * Returns size of the small square in mm
     * @return size of small the square in mm
     */
    double getSmallSquareSize() const { return m_smallSquareSize;}

    /**
     * Returns thickness of the small square in mm
     * @return thickness of small the square in mm
     */
    double getSmallSquareThickness() const { return m_smallSquareThickness;}

    /**
     * Returns size of the big square in mm
     * @return size of big the square in mm
     */
    double getBigSquareSize() const { return m_bigSquareSize;}

    /**
     * Returns thickness of the big square in mm
     * @return thickness of big the square in mm
     */
    double getBigSquareThickness() const { return m_bigSquareThickness;}

    /**
     * Returns length of the rectangle in mm
     * @return length of the rectangle in mm
     */
    double getRectangleL() const { return m_rectangleL;}

    /**
     * Returns width of the rectangle in mm
     * @return width of the rectangle in mm
     */
    double getRectangleW() const { return m_rectangleW;}

    /**
     * Returns thickness of the rectangle in mm
     * @return thickness of the rectangle in mm
     */
    double getRectangleThickness() const { return m_rectangleThickness;}

    /**
     * Returns distance from center of the rectangle in mm
     * @return distance from center of the rectangle in mm
     */
    double getRectangleDistanceFromCenter() const { return m_rectangleDistanceFromCenter; }

    /**
     * Returns vector of feb cooling configuration/geometry ID.
     * @return vector of feb cooling configuration/geometry ID.
     */
    const std::vector<double>& getFebcoolingv2GeometryID() const { return m_febcoolingv2GeometryID; }

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Upgraded after phase two cooling system (v2) geometry parameters") const;

    /**
     * Check data consistency of the cooling system (v2) positions
     * In case of failure print the BASF2 ERROR message using B2ASSERT
     */
    void checkCoolingSystemV2DataConsistency() const;

  private:

    //FEB cooling bodies.
    double m_smallSquareSize = 0.0;      /**< Size of small the square in mm */
    double m_smallSquareThickness = 0.0; /**< Thickness of small the square in mm */
    double m_bigSquareSize = 0.0;        /**< Size of big the square in mm */
    double m_bigSquareThickness = 0.0;   /**< Thickness of big the square in mm */
    double m_rectangleL = 0.0;           /**< Length of the rectangle in mm */
    double m_rectangleW = 0.0;           /**< Width of the rectangle in mm */
    double m_rectangleThickness = 0.0;   /**< Thickness of the rectangle in mm */
    double m_rectangleDistanceFromCenter = 0.0;   /**< Distance from center of the rectangle */
    /**< Feb cooling configuration/geometry ID. 0 -> Not installed, 1 -> Default position, 2 -> Rotated by 90 degree. */
    std::vector<double> m_febcoolingv2GeometryID;

    ClassDef(ARICHGeoFEBCooling, 1); /**< ClassDef */

  };

} // end namespace Belle2
