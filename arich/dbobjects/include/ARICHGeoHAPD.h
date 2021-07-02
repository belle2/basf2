/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <geometry/dbobjects/GeoOpticalSurface.h>
#include <string>

namespace Belle2 {

  /**
   * Geometry parameters of HAPD
   */
  class ARICHGeoHAPD: public ARICHGeoBase {

  public:

    /**
     * Default constructor
     */
    ARICHGeoHAPD()
    {}

    /**
     * Set wall thickness
     * @param thickness wall thickness
     */
    void setWallThickness(double thickness) {m_wallThickness = thickness;}

    /**
     * Set window thickness
     * @param thickness window thickness
     */
    void setWinThickness(double thickness) {m_winThickness = thickness;}

    /**
     * Set pad size
     * @param padSize pad size
     */
    void setPadSize(double padSize) {m_padSize = padSize;}

    /**
     * Set gap between APD chips
     * @param chipGap gap between APD chips
     */
    void setChipGap(double chipGap) {m_chipGap = chipGap;}

    /**
     * Set number of pads in x and y direction
     * @param nx number of pads in x
     * @param ny number of pads in y
     */
    void setNPads(int nx, int ny) {m_nPadX = nx; m_nPadY = ny; }

    /**
    * Set refractive index of HAPD window
    * @param refInd window refractive index
    */
    void setWinRefIndex(double refInd) {m_winRefIndex = refInd;}

    /**
     * Set module size in Z (height of HAPD + FEB)
     * @param modZ HAPD module height
     */
    void setModuleSizeZ(double modZ) {m_moduleSizeZ = modZ;}

    /**
     * Sets casing material
     * @param material casing material name
     */
    void setWallMaterial(const std::string& material) {m_wallMaterial = material;}

    /**
     * Set window material
     * @param window material name
     */
    void setWinMaterial(const std::string& material) {m_winMaterial = material;}

    /**
     * Sets APD material
     * @param APD material name
     */
    void setAPDMaterial(const std::string& material) {m_apdMaterial = material;}

    /**
     * Sets FEB material
     * @param FEB material name
     */
    void setFEBMaterial(const std::string& material) {m_febMaterial = material;}

    /**
     * Sets material inside of HAPD (vacuum with set ref. index)
     * @param fill material name
     */
    void setFillMaterial(const std::string& material) {m_fillMaterial = material;}

    /**
     * Sets FEB volume
     * @param sizeX size in x
     * @param sizeY size in y
     * @param sizeZ size in z
     * @param FEB material
     */
    void setFEBVolume(double sizeX, double sizeY, double sizeZ,
                      const std::string& material)
    {
      m_FEBSizeX = sizeX;
      m_FEBSizeY = sizeY;
      m_FEBSizeZ = sizeZ;
      m_febMaterial = material;
    }

    /**
     * Sets APD volume (single volume for all 4 chips)
     * @param sizeX size in x
     * @param sizeY size in y
     * @param sizeZ size in z
     * @param APD material name
     * @param APD optical surface properties
     */
    void setAPDVolume(double sizeX, double sizeY, double sizeZ,
                      const std::string& material, const GeoOpticalSurface& surf)
    {
      m_APDSizeX = sizeX;
      m_APDSizeY = sizeY;
      m_APDSizeZ = sizeZ;
      m_apdMaterial = material;
      m_apdSurface = surf;
    }

    /**
     * Sets HAPD volume
     * @param sizeX size in x
     * @param sizeY size in y
     * @param sizeZ size in z
     * @param fill material
     */
    void setHAPDVolume(double sizeX, double sizeY, double sizeZ,
                       const std::string& material)
    {
      m_HAPDSizeX = sizeX;
      m_HAPDSizeY = sizeY;
      m_HAPDSizeZ = sizeZ;
      m_fillMaterial = material;
    }


    /**
     * Returns HAPD size in x
     * @return HAPD size in x
     */
    double getSizeX() const {return m_HAPDSizeX / s_unit;}

    /**
     * Returns HAPD size in y
     * @return HAPD size in y
     */
    double getSizeY() const {return m_HAPDSizeY / s_unit;}

    /**
     * Returns HAPD size in z
     * @return size in z
     */
    double getSizeZ() const {return m_HAPDSizeZ / s_unit;}

    /**
     * Returns wall thickness
     * @return wall thickness
     */
    double getWallThickness() const {return m_wallThickness / s_unit;}

    /**
    * Returns window thickness
    * @return window thickness
    */
    double getWinThickness() const {return m_winThickness / s_unit;}

    /**
     * Returns APD size in x
     * @return APD size in x
     */
    double getAPDSizeX() const {return m_APDSizeX / s_unit;}

    /**
     * Returns APD size in y
     * @return APD size in y
     */
    double getAPDSizeY() const {return m_APDSizeY / s_unit;}

    /**
     * Returns APD size in z
     * @return size in z
     */
    double getAPDSizeZ() const {return m_APDSizeZ / s_unit;}

    /**
     * Returns FEB size in x
     * @return  size in x
     */
    double getFEBSizeX() const {return m_FEBSizeX / s_unit;}

    /**
     * Returns FEB size in y
     * @return size in y
     */
    double getFEBSizeY() const {return m_FEBSizeY / s_unit;}

    /**
     * Returns FEB size in z
     * @return size in z
     */
    double getFEBSizeZ() const {return m_FEBSizeZ / s_unit;}

    /**
     * Returns module size in z (HAPD + FEB height)
     * @return size in z
     */
    double getModuleSizeZ() const {return m_moduleSizeZ / s_unit;}

    /**
     * Returns window refractive index
     * @return refractive index
     */
    double getWinRefIndex() const {return m_winRefIndex;}

    /**
     * Returns APD pad size
     * @return APD pad size
     */
    double getPadSize() const {return m_padSize / s_unit;}

    /**
     * Returns gap between APD chips
     * @return chip gap size
     */
    double getChipGap() const {return m_chipGap / s_unit;}

    /**
     * Returns number of X pads
     * @return num x pads
     */
    unsigned getNumPadsX() const {return m_nPadX;}

    /**
     * Returns number of Y pads
     * @return num y pads
     */
    unsigned getNumPadsY() const {return m_nPadY;}


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
     * Returns window material name
     * @return material name
     */
    const std::string& getWinMaterial() const {return m_winMaterial;}

    /**
     * Returns APD material name
     * @return material name
     */
    const std::string& getAPDMaterial() const {return m_apdMaterial;}

    /**
     * Returns FEB material name
     * @return material name
     */
    const std::string& getFEBMaterial() const {return m_febMaterial;}

    /**
     * Returns APD reflective optical surface
     * @return optical surface
     */
    const GeoOpticalSurface& getAPDSurface() const {return m_apdSurface;}

    /**
     * Returns X and Y channel number from local x,y position
     * @param x local x position
     * @param y local y position
     * @param chX X channel
     * @param chY Y channel
     */
    void getXYChannel(double x, double y, int& chX, int& chY) const;

    /**
     * Returns APD chip ID from  from local x,y position
     * @param x local x position
     * @param y local y position
     * @return APD chip ID
     */
    unsigned getChipID(double x, double y) const;

    /**
     * Returns local x,y position from channel X,Y numbers
     * @param chX channel X number
     * @param chY channel Y number
     * @param x channel local x position
     * @param y channel local y position
     */
    void getXYChannelPos(int chX, int chY, double& x, double& y) const;


    /**
     * Returns distance between photocathode and APD
     * @return photacathod APD distance
     */
    double getPhotocathodeApdDistance() const
    {
      return (m_HAPDSizeZ - m_winThickness - m_wallThickness - m_APDSizeZ) / s_unit;
    }


    /**
     * Check consistency of geometry parameters
     * @return true if consistent
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "HAPD module geometry parameters") const override;

  private:

    std::string m_wallMaterial; /**< material of HAPD side walls */
    std::string m_winMaterial;  /**< material of HAPD window */
    std::string m_apdMaterial;  /**< material of APD chips */
    std::string m_febMaterial;  /**< material of FEB */
    std::string m_fillMaterial; /**< material that fills the HAPD */
    double m_HAPDSizeX = 0;     /**< HAPD X size */
    double m_HAPDSizeY = 0;     /**< HAPD Y size */
    double m_HAPDSizeZ = 0;     /**< HAPD Z size */
    double m_wallThickness = 0; /**< HAPD side wall thickness */
    double m_winThickness = 0;  /**< HAPD window thickness */
    double m_padSize = 0;       /**< pad size */
    double m_chipGap = 0;       /**< gap between APD chips */
    unsigned m_nPadX = 0;       /**< number of pads in X */
    unsigned m_nPadY = 0;       /**< number of pads in Y */
    double m_winRefIndex = 0;   /**< window refractive index */
    double m_APDSizeX = 0;      /**< APD X size */
    double m_APDSizeY = 0;      /**< APD Y size */
    double m_APDSizeZ = 0;      /**< APD Z size */
    double m_FEBSizeX = 0;      /**< FEB X size */
    double m_FEBSizeY = 0;      /**< FEB Y size */
    double m_FEBSizeZ = 0;      /**< FEB Z size */
    double m_moduleSizeZ = 0;   /**< HAPD module Z size (HAPD + FEB height)*/
    GeoOpticalSurface m_apdSurface; /**< optical surface of APD (for reflectivity) */

    ClassDefOverride(ARICHGeoHAPD, 1); /**< ClassDef */

  };

} // end namespace Belle2
