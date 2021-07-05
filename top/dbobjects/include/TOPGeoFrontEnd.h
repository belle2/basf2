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

namespace Belle2 {

  /**
   * Geometry parameters of board stack (front-end electronic module)
   */
  class TOPGeoFrontEnd: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoFrontEnd(): TOPGeoBase("TOPFrontEnd")
    {}

    /**
     * Sets front board data
     * @param width front board width
     * @param height front board height
     * @param thickness front board thickness
     * @param gap gap between front board and PMT array
     * @param y position of front board center in bar frame
     * @param material front board material
     */
    void setFrontBoard(double width, double height, double thickness,
                       double gap, double y, const std::string& material)
    {
      m_FBWidth = width;
      m_FBHeight = height;
      m_FBThickness = thickness;
      m_FBGap = gap;
      m_FBy = y;
      m_FBMaterial = material;
    }


    /**
     * Sets HV board data
     * @param width HV board width
     * @param length HV board length
     * @param thickness HV board thickness
     * @param gap gap between HV board and front board
     * @param y position of HV board center in bar frame
     * @param material HV board material
     */
    void setHVBoard(double width, double length, double thickness,
                    double gap, double y, const std::string& material)
    {
      m_HVWidth = width;
      m_HVLength = length;
      m_HVThickness = thickness;
      m_HVGap = gap;
      m_HVy = y;
      m_HVMaterial = material;
    }


    /**
     * Sets board stack data
     * @param width board stack width
     * @param height board stack height
     * @param length board stack length
     * @param gap gap between board stack and front board
     * @param y position of board stack center in bar frame
     * @param material board stack material
     * @param spacerWidth spacer width
     * @param spacerMaterial spacer material
     */
    void setBoardStack(double width, double height, double length,
                       double gap, double y, const std::string& material,
                       double spacerWidth, const std::string& spacerMaterial)
    {
      m_BSWidth = width;
      m_BSHeight = height;
      m_BSLength = length;
      m_BSGap = gap;
      m_BSy = y;
      m_BSMaterial = material;
      m_spacerWidth = spacerWidth;
      m_spacerMaterial = spacerMaterial;
    }

    /**
     * Returns front board width
     * @return width
     */
    double getFrontBoardWidth() const {return m_FBWidth / s_unit;}

    /**
     * Returns front board height
     * @return height
     */
    double getFrontBoardHeight() const {return m_FBHeight / s_unit;}

    /**
     * Returns front board thickness
     * @return thickness
     */
    double getFrontBoardThickness() const {return m_FBThickness / s_unit;}

    /**
     * Returns gap between front board and PMT array
     * @return gap
     */
    double getFrontBoardGap() const {return m_FBGap / s_unit;}

    /**
     * Returns position of front board center in bar frame
     * @return y
     */
    double getFrontBoardY() const {return m_FBy / s_unit;}

    /**
     * Returns front board material
     * @return material
     */
    const std::string& getFrontBoardMaterial() const {return m_FBMaterial;}


    /**
     * Returns HV board width
     * @return width
     */
    double getHVBoardWidth() const {return m_HVWidth / s_unit;}

    /**
     * Returns HV board length
     * @return length
     */
    double getHVBoardLength() const {return m_HVLength / s_unit;}

    /**
     * Returns HV board thickness
     * @return thickness
     */
    double getHVBoardThickness() const {return m_HVThickness / s_unit;}

    /**
     * Returns gap between HV board and front board
     * @return gap
     */
    double getHVBoardGap() const {return m_HVGap / s_unit;}

    /**
     * Returns position of HV board center in bar frame
     * @return y
     */
    double getHVBoardY() const {return m_HVy / s_unit;}

    /**
     * Returns HV board material
     * @return material
     */
    const std::string& getHVBoardMaterial() const {return m_HVMaterial;}


    /**
     * Returns board stack width
     * @return width
     */
    double getBoardStackWidth() const {return m_BSWidth / s_unit;}

    /**
     * Returns board stack height
     * @return height
     */
    double getBoardStackHeight() const {return m_BSHeight / s_unit;}

    /**
     * Returns board stack length
     * @return length
     */
    double getBoardStackLength() const {return m_BSLength / s_unit;}

    /**
     * Returns gap between board stack and front board
     * @return gap
     */
    double getBoardStackGap() const {return m_BSGap / s_unit;}

    /**
     * Returns position of board stack center in bar frame
     * @return y
     */
    double getBoardStackY() const {return m_BSy / s_unit;}

    /**
     * Returns board stack material
     * @return material
     */
    const std::string& getBoardStackMaterial() const {return m_BSMaterial;}

    /**
     * Returns board stack spacer width
     * @return width
     */
    double getSpacerWidth() const {return m_spacerWidth / s_unit;}

    /**
     * Returns board stack spacer material
     * @return material
     */
    const std::string& getSpacerMaterial() const {return m_spacerMaterial;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    virtual void print(const std::string& title = "Front-end geometry parameters") const override;

  private:

    float m_FBWidth = 0;      /**< front board width */
    float m_FBHeight = 0;     /**< front board height */
    float m_FBThickness = 0;  /**< front board thickness */
    float m_FBGap = 0;        /**< gap between front board and PMT array */
    float m_FBy = 0;          /**< position of front board center in bar frame */
    std::string m_FBMaterial; /**< front board material */

    float m_HVWidth = 0;      /**< HV board width */
    float m_HVLength = 0;     /**< HV board length */
    float m_HVThickness = 0;  /**< HV board thickness */
    float m_HVGap = 0;        /**< gap between HV board and front board */
    float m_HVy = 0;          /**< position of HV board center in bar frame */
    std::string m_HVMaterial; /**< HV board material */

    float m_BSWidth = 0;      /**< board stack width */
    float m_BSHeight = 0;     /**< board stack height */
    float m_BSLength = 0;     /**< board stack length */
    float m_BSGap = 0;        /**< gap between board stack and front board */
    float m_BSy = 0;          /**< position of board stack center in bar frame */
    std::string m_BSMaterial; /**< board stack material */
    float m_spacerWidth = 0;  /**< board stack spacer width */
    std::string m_spacerMaterial; /**< board stack spacer material */

    ClassDefOverride(TOPGeoFrontEnd, 1); /**< ClassDef */

  };

} // end namespace Belle2
