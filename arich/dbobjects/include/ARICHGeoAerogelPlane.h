/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Leonid Burmistrov                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>
#include <TVector3.h>
#include <TVector2.h>
#include <TRotation.h>

namespace Belle2 {

  /**
   * Geometry parameters of HAPD
   */

  class ARICHGeoAerogelPlane: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHGeoAerogelPlane()
    {}

    /**
     * Struct to hold aerogel layer parameters
     * Only for averaged properties of the aerogel tiles/layers
     */
    struct layer {
      double thickness;
      double refIndex;
      double trLength;
      std::string material;
    };

    /**
     * Struct to hold individual aerogel tile parameters
     * layer : 0 - up
     * layer : 1 - down
     */
    struct tilestr {
      int ring;
      int column;
      int layer;
      double n;
      double transmL;
      double thick;
      std::string material;
    };

    /**
     * Consistency check of geometry parameters
     * @return true if consistent
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Aerogel plane parameters:") const;

    /**
     * Set aerogel plane positioning within ARICH local volume
     * @param x x position
     * @param y y position
     * @param z z position
     * @param rx rotation around x-axis
     * @param ry rotation around y-axis
     * @param rz rotation around z-axis
     */
    void setPlacement(double x, double y, double z, double rx, double ry, double rz) {m_x = x; m_y = y; m_z = z; m_rx = rx; m_ry = ry; m_rz = rz;}

    /**
     * Set parameters of aerogel support plate
     * @param inR inner radius
     * @param outR outer radius
     * @param thick thickness
     * @param material material
     */
    void addSupportPlate(double inR, double outR, double thick, const std::string& material)
    {
      m_innerR = inR;
      m_outerR = outR;
      m_thickness = thick;
      m_supportMaterial = material;
    };

    /**
     * Set thickness of aluminum walls between aerogel tiles
     * @param thick wall thickness
     */
    void setWallThickness(double thick) {m_wallThickness = thick;}

    /**
     * Set height of aluminum walls between aerogel tiles
     * @param height wall height
     */
    void setWallHeight(double height) {m_wallHeight = height;}

    /**
     * Set gap between aerogel tile and aluminum wall
     * @param gap gap size
     */
    void setTileGap(double gap) {m_tileGap = gap;}

    /**
     * Add parameters of individual tile
     * @param ring aerogel tile ring ID (range : [1;4])
     * @param column aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40])
     * @param layer aerogel tile layer ID (layer : 0 - up; layer : 1 - down)
     * @param n aerogel tile refractive index
     * @param transmL aerogel tile rayleigh scattering length
     * @param thick aerogel tile thickness
     * @param material aerogel tile material name
     */
    void addTileParameters(int ring, int column, int layerN, double n, double transmL, double thick, const std::string& material)
    {
      m_tiles.push_back({ ring, column, layerN, n, transmL, thick, material });
    }

    /**
     * get position vector of aerogel plane in ARICH local frame
     * @return position of aerogel plane center point (TVector3)
     */
    TVector3 getPosition() const {return TVector3(m_x / s_unit, m_y / s_unit, m_z / s_unit);}

    /**
     * get rotation matrix of aerogel plane in ARICH local frame
     * @return rotation matrix of aerogel plane (TRotation)
     */
    TRotation getRotation() const
    {
      TRotation rot;
      rot.RotateX(m_rx).RotateY(m_ry).RotateZ(m_rz);
      return rot;
    }

    /**
     * Get angle of rotation around X axis
     * @return rotation angle arounx X axis
     */
    double getRotationX() const {return m_rx;}

    /**
     * Get angle of rotation around Y axis
     * @return rotation angle arounx Y axis
     */
    double getRotationY() const {return m_ry;}

    /**
     * Get angle of rotation around Z axis
     * @return rotation angle arounx Z axis
     */
    double getRotationZ() const {return m_rz;}

    /**
     * Get support plate inner radius
     * @return support plate inner radius
     */
    double getSupportInnerR() const {return m_innerR / s_unit;};

    /**
     * Get support plate outer radius
     * @return support plate outer radius
     */
    double getSupportOuterR() const {return m_outerR / s_unit;};

    /**
     * Get support plate thickness
     * @return support plate thickness
     */
    double getSupportThickness() const {return m_thickness / s_unit;};

    /**
     * Get thickness of aluminum walls between aerogel tiles
     * @return thickness of aluminum walls
     */
    double getWallThickness() const {return m_wallThickness / s_unit;};

    /**
     * Get height of aluminum walls between aerogel tiles
     * @return height of aluminum walls
     */
    double getWallHeight() const {return m_wallHeight / s_unit;};

    /**
     * Get gap between aerogel tile and aluminum wall
     * @return gap between aerogel tile and aluminum wall
     */
    double getTileGap() const {return m_tileGap / s_unit;};

    /**
     * Get material of support plate
     * @return support plate material name
     */
    const std::string& getSupportMaterial() const {return m_supportMaterial;}

    /**
     * Set radiuses at which "r" aluminum walls between tiles are placed (+inner+outter aluminum ring)
     * @param rs vector of aluminum rings between aerogel tiles radiuses (inner radius!)
     */
    void setWallRadius(std::vector<double>& rs) { m_r = rs; m_rSize = rs.size();}

    /**
     * Set phi (angle) distance between "phi" aluminum walls between aerogel tiles for all aerogel tile rings
     * @param dphi vector of phi (angle) distances
     */
    void setWallDPhi(std::vector<double>& dphi) { m_dPhi = dphi; m_dPhiSize = dphi.size();}

    /**
     * Set parameters of i-th aerogel layer
     * @param ilayer layer number (1 is placed first, at lowest z, etc...)
     * @param thick thickness of aerogel tiles in layer
     * @param rIndex refractive index of aerogel tiles in layer
     * @param trLen transmission length of aerogel tiles in layer
     * @param material material of aerogel tiles in layer
     */
    void setAerogelLayer(unsigned ilayer, double thick, double rIndex, double trLen, std::string& material)
    {
      if (ilayer == m_nLayers + 1) {
        m_layers.push_back({thick, rIndex, trLen, material});
        m_nLayers++;
      } else if (ilayer < m_nLayers + 1) {
        m_layers[ilayer - 1] = {thick, rIndex, trLen, material};
      } else B2ERROR("ARICHGeoAerogelPlane::setAerogelLayer: please set aerogel layers in consecutive order!");
    }

    /**
     * Set vector of numbers of aerogel slots in individual ring
     * @param nAeroSlotsIndividualRing vector of numbers of aerogel slots in individual ring
     */
    void setNAeroSlotsIndividualRing(std::vector<int>& nAeroSlotsIndividualRing) { m_nAeroSlotsIndividualRing = nAeroSlotsIndividualRing; }

    /**
     * Get number of aerogel layers
     * @return number of aerogel layers
     */
    unsigned getNLayers() const {return m_nLayers;}

    /**
     * Get number of aluminum wall rings (should be number of tile rings + 1)
     * @return number of aluminum wall rings
     */
    unsigned getNRings() const {return m_rSize;}

    /**
     * Get radius of i-th aluminum ring between aerogel tiles (inner radius of ring)
     * @param iRing ring number (from 1->)
     * @return radius of i-th aluminum ring
     */
    double getRingRadius(unsigned iRing) const { if (iRing > m_rSize || iRing == 0) B2ERROR("ARICHGeoAerogelPlane: invalid ring number!"); return m_r[iRing - 1] / s_unit;}

    /**
     * Get phi (angle) distance between "phi" aluminum wall between aerogel tiles in i-th tile ring
     * @param iRing ring number (from 1->)
     * @return phi distance between aluminum walls in aerogel tiles ring
     */
    double getRingDPhi(unsigned iRing) const { if (iRing > m_rSize || iRing == 0) B2ERROR("ARICHGeoAerogelPlane: invalid ring number!"); return m_dPhi[iRing - 1];}

    /**
     * Get thickness of tiles i-th aerogel layer
     * @param iLayer layer number
     * @return thickness of tiles in layer
     */
    double getLayerThickness(unsigned iLayer) const {if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].thickness / s_unit;}

    /**
     * Get refractive index of tiles i-th aerogel layer
     * @param iLayer layer number
     * @return refractive index of tiles in layer
     */
    double getLayerRefIndex(unsigned iLayer) const { if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].refIndex;}

    /**
     * Get transmission length of tiles i-th aerogel layer
     * @param iLayer layer number
     * @return transmission length of tiles in layer
     */
    double getLayerTrLength(unsigned iLayer) const { if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].trLength / s_unit;}

    /**
     * Get material name of tiles i-th aerogel layer
     * @param iLayer layer number
     * @return material name of tiles in layer
     */
    const std::string& getLayerMaterial(unsigned iLayer) const { if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].material;}

    /**
     * Get ID of aerogel tile containing point (x,y) (actually this is tile slot ID, as it is the same for all layers)
     * @param x x position
     * @param y y position
     * @return aerogel tile ID
     */
    unsigned getAerogelTileID(double x, double y) const;

    /**
     * Set to use simple aerogel plane (single square aerogel tile (2 layers), for cosmic test for example)
     * Vector of parameters should have the for of: x size, y size, x position, y position, rotation phi angle
     * @param params vector of aerogel tile parameters
     */
    void setSimple(std::vector<double>& params)
    {
      if (params.size() != 5) B2ERROR("ARICHGeoAerogelPlane:setSimple: 5 parameters are needed for simple configuration!");
      m_simple = true;
      m_simpleParams = params;
    }

    /**
     * Use simple aerogel configuration
     * @return true if simple
     */
    bool isSimple() const
    {
      return m_simple;
    }

    /**
     * Get parameters of simple aerogel configuration
     * @return parametrs
     */
    const std::vector<double>& getSimpleParams() const
    {
      return m_simpleParams;
    }

    /**
     * Get vector of numbers of aerogel slots in individual ring
     * @param nAeroSlotsIndividualRing vector of numbers of aerogel slots in individual ring
     */
    const std::vector<int>& getNAeroSlotsIndividualRing() const
    {
      return m_nAeroSlotsIndividualRing;
    }

    /**
     * Get starting Z position of first aerogel layer
     * @return starting z
     */
    double getAerogelZPosition() const
    {
      return m_z / s_unit - (m_wallHeight / s_unit - m_thickness / s_unit) / 2.;
    }

    /**
     * Set full aerogel material description key.
     * 1 - use material explicitly for each aerogel tile.
     * Any integer (but not 1) - uses two types of aerogel material for upstream and downstream.
     * @param fullAerogelMaterialDescriptionKey - full aerogel material description key.
     */
    void setFullAerogelMaterialDescriptionKey(int fullAerogelMaterialDescriptionKey)
    {
      m_fullAerogelMaterialDescriptionKey = fullAerogelMaterialDescriptionKey;
    }

    /**
     * Get full aerogel material description key.
     * 1 - use material explicitly for each aerogel tile.
     * Any integer (but not 1) - uses two types of aerogel material for upstream and downstream.
     * @return full aerogel material description key
     */
    int getFullAerogelMaterialDescriptionKey() const
    {
      return m_fullAerogelMaterialDescriptionKey;
    }

    /**
     * Get parameters of individual tile
     * @param ring aerogel tile ring ID (range : [1;4])
     * @param column aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40])
     * @param layer aerogel tile layer ID (layer : 0 - up; layer : 1 - down)
     * @param &n address of the variable : n aerogel tile refractive index
     * @param &transmL address of the variable : transmL aerogel tile rayleigh scattering length
     * @param &thick address of the variable : thick aerogel tile thickness
     * @param &material address of the variable : material aerogel tile material name
     * @return entry id number
     */
    unsigned getTileParameters(int ring, int column, int layerN, double& n, double& transmL, double& thick,
                               std::string& material) const;

    /**
     * Get thickness of individual tile
     * @param ring aerogel tile ring ID (range : [1;4])
     * @param column aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40])
     * @param layer aerogel tile layer ID (layer : 0 - up; layer : 1 - down)
     * @return thickness of individual tile
     */
    double getTileThickness(int ring, int column, int layerN) const;

    /**
     * Get material name of individual tile
     * @param ring aerogel tile ring ID (range : [1;4])
     * @param column aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40])
     * @param layer aerogel tile layer ID (layer : 0 - up; layer : 1 - down)
     * @return material name of individual tile
     */
    std::string getTileMaterialName(int ring, int column, int layerN) const;

    /**
     * Get total thickness of the aerogel tiles tile_up + tile_down for a given slot
     * @param ring aerogel tile ring ID
     * @param column aerogel tile column ID
     * @return total thickness of the aerogel tiles tile_up + tile_down
     */
    double getTotalTileThickness(int ring, int column) const;

    /**
     * Get maximum total thickness of the aerogel tiles tile_up + tile_down for all the slots
     * @return maximum total thickness of the aerogel tiles tile_up + tile_down for all the slots
     */
    double getMaximumTotalTileThickness() const;

    /**
     * Set imaginary tube thikness just after aerogel layers used
     * as volume to which tracks are extrapolated.
     * @param imgTubeThickness imaginary tube thikness.
     */
    void setImgTubeThickness(double imgTubeThickness)
    {
      m_imgTubeThickness = imgTubeThickness;
    }

    /**
     * Get imaginary tube thikness just after aerogel layers used
     * as volume to which tracks are extrapolated.
     * @return imaginary tube thikness
     */
    double getImgTubeThickness() const
    {
      return m_imgTubeThickness;
    }

    /**
     * Set minimum thickness of the compensation volume with ARICH air
     * @param compensationARICHairVolumeThick_min minimum thickness of the compensation volume with ARICH air.
     */
    void setCompensationARICHairVolumeThick_min(double compensationARICHairVolumeThick_min)
    {
      m_compensationARICHairVolumeThick_min = compensationARICHairVolumeThick_min;
    }

    /**
     * Get minimum thickness of the compensation volume with ARICH air
     * @return thickness of the compensation volume
     */
    double getCompensationARICHairVolumeThick_min() const
    {
      return m_compensationARICHairVolumeThick_min;
    }

    /**
     * Print the content of the m_tiles vector of tilestr structure
     * it contains position of the tile its refractive index, rayleigh scattering length, thickness
     * @param title title to be printed
     */
    void printTileParameters(const std::string& title = "Aerogel tiles parameters:") const;

    /**
     * Print the content of the single tilestr structure
     * it contains position of the tile its refractive index, rayleigh scattering length, thickness
     * @param i entry counter
     */
    void printSingleTileParameters(unsigned i) const;

    /**
     * This function tests the getTileParameters function
     */
    void testGetTileParametersFunction() const;

  private:

    // position in local ARICH volume
    double m_x = 0;               /**< x position */
    double m_y = 0;               /**< y position */
    double m_z = 0;               /**< z position */
    // rotations in local ARICH volume
    double m_rx = 0;              /**< rotation around x-axis */
    double m_ry = 0;              /**< rotation around y-axis */
    double m_rz = 0;              /**< rotation around z-axis */

    std::vector<double> m_r;      /**< "r" aluminum wall radiuses */
    std::vector<double> m_dPhi;   /**< "phi" aluminum wall distances in tile ring */
    std::vector<int> m_nAeroSlotsIndividualRing; /**< Number of aerogel slots in individual ring */

    std::vector<layer> m_layers;  /**< parameters averaged properties of the aerogel tiles/layers (up and down) */
    std::vector<tilestr> m_tiles; /**< parameters of the individual aerogel tiles */

    double m_tileGap = 0;         /**< gap between aerogel tiles and aluminum walls */

    // support structure parameters
    std::string m_supportMaterial; /**< material of support plate */
    double m_innerR = 0;           /**< inner radius of support plate */
    double m_outerR = 0;           /**< outer radius of support plate */
    double m_thickness = 0;        /**< thickness of support plate */
    double m_wallThickness = 0;    /**< thickness of aluminum walls between aerogel tiles */
    double m_wallHeight = 0;       /**< height (z) of aluminum walls between aerogel tiles */

    unsigned m_rSize = 0;          /**< size of m_r vector */
    unsigned m_dPhiSize = 0;       /**< size of m_dPhi vector */
    unsigned m_nLayers = 0;        /**< number of aerogel tile layers */

    bool m_simple = 0;
    std::vector<double> m_simpleParams;

    int m_fullAerogelMaterialDescriptionKey =
      0; /**< Full aerogel material description key : 1 - use material explicitly for each aerogel tile, 0 - use two types of aerogel material */

    double m_imgTubeThickness =
      0.0; /**< imaginary tube thikness just after aerogel layers used as volume to which tracks are extrapolated */

    double m_compensationARICHairVolumeThick_min; /**< Minimum thickness of the compensation volume with ARICH air */

    ClassDef(ARICHGeoAerogelPlane, 3); /**< ClassDef */

  };

} // end namespace Belle2
