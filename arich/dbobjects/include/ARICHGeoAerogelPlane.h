/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHGeoBase.h>
#include <string>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {

  /**
   * Geometry parameters of HAPD.
   */
  class ARICHGeoAerogelPlane: public ARICHGeoBase {

  public:

    /**
     * Default constructor.
     */
    ARICHGeoAerogelPlane()
    {}

    /**
     * Struct to hold aerogel layer parameters.
     * Only for averaged properties of the aerogel tiles/layers.
     */
    struct layer {

      /** Thickness. */
      double thickness;

      /** Refractive index. */
      double refIndex;

      /** Transmission length. */
      double trLength;

      /** Material. */
      std::string material;

    };

    /**
     * Struct to hold individual aerogel tile parameters.
     * Layer : 0 - up.
     * Layer : 1 - down.
     */
    struct tilestr {

      /** Ring. */
      int ring;

      /** Column. */
      int column;

      /** Layer. */
      int layer;

      /** Refractive index. */
      double n;

      /** Transmission length. */
      double transmL;

      /** Thickness. */
      double thick;

      /** Material. */
      std::string material;

    };

    /**
     * Consistency check of geometry parameters.
     * @return true if consistent
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class.
     * @param[in] title Title to be printed.
     */
    void print(const std::string& title = "Aerogel plane parameters:") const override;

    /**
     * Set aerogel plane positioning within ARICH local volume.
     * @param[in] x  X position.
     * @param[in] y  Y position.
     * @param[in] z  Z position.
     * @param[in] rx Rotation around x-axis.
     * @param[in] ry Rotation around y-axis.
     * @param[in] rz Rotation around z-axis.
     */
    void setPlacement(double x, double y, double z, double rx, double ry, double rz) {m_x = x; m_y = y; m_z = z; m_rx = rx; m_ry = ry; m_rz = rz;}

    /**
     * Set parameters of aerogel support plate.
     * @param[in] inR      Inner radius.
     * @param[in] outR     Outer radius.
     * @param[in] thick    Thickness.
     * @param[in] material Material.
     */
    void addSupportPlate(double inR, double outR, double thick, const std::string& material)
    {
      m_innerR = inR;
      m_outerR = outR;
      m_thickness = thick;
      m_supportMaterial = material;
    };

    /**
     * Set thickness of aluminum walls between aerogel tiles.
     * @param[in] thick Wall thickness.
     */
    void setWallThickness(double thick) {m_wallThickness = thick;}

    /**
     * Set height of aluminum walls between aerogel tiles
     * @param[in] height Wall height.
     */
    void setWallHeight(double height) {m_wallHeight = height;}

    /**
     * Set gap between aerogel tile and aluminum wall
     * @param[in] gap Gap size.
     */
    void setTileGap(double gap) {m_tileGap = gap;}

    /**
     * Add parameters of individual tile
     *
     * @param[in] ring
     * Aerogel tile ring ID (range : [1;4]).
     *
     * @param[in] column
     * Aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40]).
     *
     * @param[in] layerN
     * Aerogel tile layer ID (layer : 0 - up; layer : 1 - down).
     *
     * @param[in] n
     * Aerogel tile refractive index.
     *
     * @param[in] transmL
     * Aerogel tile rayleigh scattering length.
     *
     * @param[in] thick
     * Aerogel tile thickness.
     *
     * @param[in] material
     * Aerogel tile material name.
     */
    void addTileParameters(int ring, int column, int layerN, double n, double transmL, double thick, const std::string& material)
    {
      m_tiles.push_back({ ring, column, layerN, n, transmL, thick, material });
    }

    /**
     * Get position vector of aerogel plane in ARICH local frame.
     * @return Position of aerogel plane center point (TVector3).
     */
    TVector3 getPosition() const {return TVector3(m_x / s_unit, m_y / s_unit, m_z / s_unit);}

    /**
     * Get rotation matrix of aerogel plane in ARICH local frame.
     * @return Rotation matrix of aerogel plane (TRotation).
     */
    TRotation getRotation() const
    {
      TRotation rot;
      rot.RotateX(m_rx).RotateY(m_ry).RotateZ(m_rz);
      return rot;
    }

    /**
     * Get angle of rotation around X axis.
     * @return Rotation angle arounx X axis.
     */
    double getRotationX() const {return m_rx;}

    /**
     * Get angle of rotation around Y axis.
     * @return Rotation angle arounx Y axis.
     */
    double getRotationY() const {return m_ry;}

    /**
     * Get angle of rotation around Z axis.
     * @return Rotation angle arounx Z axis.
     */
    double getRotationZ() const {return m_rz;}

    /**
     * Get support-plate inner radius.
     * @return Support-plate inner radius.
     */
    double getSupportInnerR() const {return m_innerR / s_unit;};

    /**
     * Get support-plate outer radius.
     * @return Support-plate outer radius.
     */
    double getSupportOuterR() const {return m_outerR / s_unit;};

    /**
     * Get support-plate thickness.
     * @return Support-plate thickness.
     */
    double getSupportThickness() const {return m_thickness / s_unit;};

    /**
     * Get thickness of aluminum walls between aerogel tiles.
     * @return Thickness of aluminum walls.
     */
    double getWallThickness() const {return m_wallThickness / s_unit;};

    /**
     * Get height of aluminum walls between aerogel tiles.
     * @return Height of aluminum walls.
     */
    double getWallHeight() const {return m_wallHeight / s_unit;};

    /**
     * Get gap between aerogel tile and aluminum wall.
     * @return Gap between aerogel tile and aluminum wall.
     */
    double getTileGap() const {return m_tileGap / s_unit;};

    /**
     * Get material of support plate.
     * @return Support-plate material name.
     */
    const std::string& getSupportMaterial() const {return m_supportMaterial;}

    /**
     * Set radiuses at which "r" aluminum walls between tiles are placed
     * (+inner+outter aluminum ring).
     *
     * @param[in] rs
     * Vector of aluminum rings between aerogel tiles radiuses (inner radius!).
     */
    void setWallRadius(std::vector<double>& rs) { m_r = rs; m_rSize = rs.size();}

    /**
     * Set phi (angle) distance between "phi" aluminum walls between aerogel
     * tiles for all aerogel tile rings.
     * @param[in] dphi Vector of phi (angle) distances.
     */
    void setWallDPhi(std::vector<double>& dphi) { m_dPhi = dphi; m_dPhiSize = dphi.size();}

    /**
     * Set parameters of i-th aerogel layer.
     *
     * @param[in] ilayer
     * Layer number (1 is placed first, at lowest z, etc...).
     *
     * @param[in] thick
     * Thickness of aerogel tiles in layer.
     *
     * @param[in] rIndex
     * Refractive index of aerogel tiles in layer.
     *
     * @param[in] trLen
     * Transmission length of aerogel tiles in layer
     *
     * @param[in] material
     * Material of aerogel tiles in layer
     */
    void setAerogelLayer(unsigned ilayer, double thick, double rIndex, double trLen, const std::string& material)
    {
      if (ilayer == m_nLayers + 1) {
        m_layers.push_back({thick, rIndex, trLen, material});
        m_nLayers++;
      } else if (ilayer < m_nLayers + 1) {
        m_layers[ilayer - 1] = {thick, rIndex, trLen, material};
      } else B2ERROR("ARICHGeoAerogelPlane::setAerogelLayer: please set aerogel layers in consecutive order!");
    }

    /**
     * Set vector of numbers of aerogel slots in individual ring.
     *
     * @param[in] nAeroSlotsIndividualRing
     * Vector of numbers of aerogel slots in individual ring.
     */
    void setNAeroSlotsIndividualRing(const std::vector<int>& nAeroSlotsIndividualRing) { m_nAeroSlotsIndividualRing = nAeroSlotsIndividualRing; }

    /**
     * Get number of aerogel layers.
     * @return Number of aerogel layers.
     */
    unsigned getNLayers() const {return m_nLayers;}

    /**
     * Get number of aluminum wall rings (should be number of tile rings + 1).
     * @return Number of aluminum wall rings.
     */
    unsigned getNRings() const {return m_rSize;}

    /**
     * Get radius of i-th aluminum ring between aerogel tiles
     * (inner radius of ring).
     * @param[in] iRing Ring number (from 1->).
     * @return Radius of i-th aluminum ring.
     */
    double getRingRadius(unsigned iRing) const { if (iRing > m_rSize || iRing == 0) B2ERROR("ARICHGeoAerogelPlane: invalid ring number!"); return m_r[iRing - 1] / s_unit;}

    /**
     * Get phi (angle) distance between "phi" aluminum wall between aerogel
     * tiles in i-th tile ring
     * @param[in] iRing Ring number (from 1->).
     * @return Phi distance between aluminum walls in aerogel tiles ring.
     */
    double getRingDPhi(unsigned iRing) const { if (iRing > m_rSize || iRing == 0) B2ERROR("ARICHGeoAerogelPlane: invalid ring number!"); return m_dPhi[iRing - 1];}

    /**
     * Get thickness of tiles in i-th aerogel layer.
     * @param[in] iLayer Layer number.
     * @return Thickness of tiles in layer.
     */
    double getLayerThickness(unsigned iLayer) const {if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].thickness / s_unit;}

    /**
     * Get refractive index of tiles in i-th aerogel layer.
     * @param[in] iLayer Layer number.
     * @return Refractive index of tiles in layer.
     */
    double getLayerRefIndex(unsigned iLayer) const { if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].refIndex;}

    /**
     * Get transmission length of tiles in i-th aerogel layer.
     * @param[in] iLayer Layer number.
     * @return Transmission length of tiles in layer.
     */
    double getLayerTrLength(unsigned iLayer) const { if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].trLength / s_unit;}

    /**
     * Get material name of tiles in i-th aerogel layer.
     * @param[in] iLayer Layer number.
     * @return Material name of tiles in layer.
     */
    const std::string& getLayerMaterial(unsigned iLayer) const { if (iLayer > m_nLayers || iLayer == 0) B2ERROR("ARICHGeoAerogelPlane: invalid aerogel layer number!"); return m_layers[iLayer - 1].material;}

    /**
     * Get ID of aerogel tile containing point (x,y)
     * (actually this is tile slot ID, as it is the same for all layers).
     * @param[in] x X position.
     * @param[in] y Y position.
     * @return Aerogel tile ID.
     */
    unsigned getAerogelTileID(double x, double y) const;

    /**
     * Set to use simple aerogel plane (single square aerogel tile (2 layers),
     * for cosmic test for example). Vector of parameters should contain:
     * x size, y size, x position, y position, rotation phi angle.
     * @param[in] params Vector of aerogel tile parameters.
     */
    void setSimple(std::vector<double>& params)
    {
      if (params.size() != 5) B2ERROR("ARICHGeoAerogelPlane:setSimple: 5 parameters are needed for simple configuration!");
      m_simple = true;
      m_simpleParams = params;
    }

    /**
     * Use simple aerogel configuration.
     * @return True if simple.
     */
    bool isSimple() const
    {
      return m_simple;
    }

    /**
     * Get parameters of simple aerogel configuration.
     * @return Parameters.
     */
    const std::vector<double>& getSimpleParams() const
    {
      return m_simpleParams;
    }

    /**
     * Get vector of numbers of aerogel slots in individual ring.
     *
     * @param[in] nAeroSlotsIndividualRing
     * Vector of numbers of aerogel slots in individual ring.
     */
    const std::vector<int>& getNAeroSlotsIndividualRing() const
    {
      return m_nAeroSlotsIndividualRing;
    }

    /**
     * Get starting Z position of first aerogel layer.
     * @return Starting z.
     */
    double getAerogelZPosition() const
    {
      return m_z / s_unit - (m_wallHeight / s_unit - m_thickness / s_unit) / 2.;
    }

    /**
     * Set full aerogel material description key.
     * 1 - use material explicitly for each aerogel tile.
     * Any integer (but not 1) - uses two types of aerogel material
     * for upstream and downstream.
     *
     * @param[in] fullAerogelMaterialDescriptionKey
     * Full aerogel material description key.
     */
    void setFullAerogelMaterialDescriptionKey(int fullAerogelMaterialDescriptionKey)
    {
      m_fullAerogelMaterialDescriptionKey = fullAerogelMaterialDescriptionKey;
    }

    /**
     * Get full aerogel material description key.
     * 1 - use material explicitly for each aerogel tile.
     * Any integer (but not 1) - uses two types of aerogel material
     * for upstream and downstream.
     * @return Full aerogel material description key.
     */
    int getFullAerogelMaterialDescriptionKey() const
    {
      return m_fullAerogelMaterialDescriptionKey;
    }

    /**
     * Get parameters of individual tile
     *
     * @param[in] ring
     * Aerogel tile ring ID (range : [1;4]).
     *
     * @param[in] column
     * Aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40]).
     *
     * @param[in] layerN
     * Aerogel tile layer ID (layer : 0 - up; layer : 1 - down).
     *
     * @param[in] n
     * Aerogel tile refractive index.
     *
     * @param[in] transmL
     * Aerogel tile rayleigh scattering length.
     *
     * @param[in] thick
     * Aerogel tile thickness.
     *
     * @param[in] material
     * Aerogel tile material name.
     *
     * @return Entry id number.
     */
    unsigned getTileParameters(int ring, int column, int layerN, double& n, double& transmL, double& thick,
                               std::string& material) const;

    /**
     * Get thickness of individual tile.
     *
     * @param[in] ring
     * Aerogel tile ring ID (range : [1;4]).
     *
     * @param[in] column
     * Aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40]).
     *
     * @param[in] layerN
     * Aerogel tile layer ID (layer : 0 - up; layer : 1 - down).
     *
     * @return Thickness of individual tile.
     */
    double getTileThickness(int ring, int column, int layerN) const;

    /**
     * Get material name of individual tile.
     *
     * @param[in] ring
     * Aerogel tile ring ID (range : [1;4]).
     *
     * @param[in] column
     * Aerogel tile column ID (range : [1;22] [1;28] [1;34] [1;40]).
     *
     * @param[in] layerN
     * Aerogel tile layer ID (layer : 0 - up; layer : 1 - down).
     *
     * @return Material name of individual tile.
     */
    std::string getTileMaterialName(int ring, int column, int layerN) const;

    /**
     * Get total thickness of the aerogel tiles tile_up + tile_down
     * for a given slot.
     * @param[in] ring   Aerogel tile ring ID.
     * @param[in] column Aerogel tile column ID,
     * @return Total thickness of the aerogel tiles tile_up + tile_down.
     */
    double getTotalTileThickness(int ring, int column) const;

    /**
     * Get maximum total thickness of the aerogel tiles tile_up + tile_down
     * for all the slots.
     * @return Maximum total thickness of the aerogel tiles tile_up + tile_down
     * for all the slots.
     */
    double getMaximumTotalTileThickness() const;

    /**
     * Set imaginary tube thikness just after aerogel layers used
     * as volume to which tracks are extrapolated.
     * @param[in] imgTubeThickness imaginary tube thikness.
     */
    void setImgTubeThickness(double imgTubeThickness)
    {
      m_imgTubeThickness = imgTubeThickness;
    }

    /**
     * Get imaginary tube thikness just after aerogel layers used
     * as volume to which tracks are extrapolated.
     * @return Imaginary tube thikness.
     */
    double getImgTubeThickness() const
    {
      return m_imgTubeThickness;
    }

    /**
     * Set minimum thickness of the compensation volume with ARICH air.
     *
     * @param[in] compensationARICHairVolumeThick_min
     * Minimum thickness of the compensation volume with ARICH air.
     */
    void setCompensationARICHairVolumeThick_min(double compensationARICHairVolumeThick_min)
    {
      m_compensationARICHairVolumeThick_min = compensationARICHairVolumeThick_min;
    }

    /**
     * Get minimum thickness of the compensation volume with ARICH air.
     * @return Thickness of the compensation volume.
     */
    double getCompensationARICHairVolumeThick_min() const
    {
      return m_compensationARICHairVolumeThick_min;
    }

    /**
     * Print the content of the m_tiles vector of tilestr structure.
     * It contains position of the tile its refractive index,
     * Rayleigh scattering length, thickness.
     * @param[in] title Title to be printed.
     */
    void printTileParameters(const std::string& title = "Aerogel tiles parameters:") const;

    /**
     * Print the content of the single tilestr structure.
     * It contains position of the tile its refractive index,
     * Rayleigh scattering length, thickness.
     * @param[in] i Entry counter.
     */
    void printSingleTileParameters(unsigned i) const;

    /**
     * This function tests the getTileParameters function.
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

    double m_compensationARICHairVolumeThick_min = 0; /**< Minimum thickness of the compensation volume with ARICH air */

    ClassDefOverride(ARICHGeoAerogelPlane, 3); /**< ClassDef */

  };

} // end namespace Belle2
