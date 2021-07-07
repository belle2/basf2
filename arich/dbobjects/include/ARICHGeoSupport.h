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

namespace Belle2 {

  /**
   * Geometry parameters of ARICH support structures and neutron shield
   */

  class ARICHGeoSupport: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHGeoSupport()
    {}

    /**
     * Struct to hold tube parameters
     */
    struct tube {
      double innerR;
      double outerR;
      double length;
      double zPosition;
      std::string material;
      std::string name;
    };

    /**
     * Struct to hold wedge parameters
     */
    struct wedge {
      int type;
      double r;
      double phi;
      double z;
      std::string material;
    };

    /**
     * Struct to hold parameters of box volumes (examples, scintilators for cosmic test)
     */
    struct box {
      double size[3];
      double position[3];
      double rotation[3];
      std::string name;
      std::string material;
    };


    /**
     * Consistency check of geometry parameters
     * @return true if consistent
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Parameters of ARICH support structure and neutron shield") const override;

    /**
     * Set parameters of support wedge volumes
     * @param i wedge type ID (1-3)
     * @param par vector of wedge parameters
     */
    void setWedge(unsigned i, const std::vector<double>& par) {if (i > m_nWedgeType || i == 0) { B2ERROR("invalid wedge number"); return;}; m_wedgePar[i - 1] = par;};

    /**
     * Add wedge (all added wedges are placed)
     * @param type wedge type ID (1-3)
     * @param r radius at which wedge is placed
     * @param phi phi angle at which wedge is placed
     * @param z z position of wedge (in ARICH frame)
     * @param material wedge material
     */
    void addWedge(int type, double r, double phi, double z, const std::string& material)
    {
      m_wedges.push_back({type, r, phi, z, material});
    }

    /**
     * Set default material of support structures
     * @param material name of default material
     */
    void setMaterial(const std::string& material) { m_material = material;};

    /**
     * Add volume of cylindrical shape (tube)
     * @param inR inner radius
     * @param outR outer radius
     * @param length length
     * @param zPosition Z position of start of tube (in ARICH frame)
     */
    void addTube(double inR, double outR, double length, double zPosition, const std::string& material, const std::string& name)
    {
      m_tubes.push_back({inR, outR, length, zPosition, material, name});
    }

    /**
     * Get number of tube volumes to be placed
     * @return number of tube volumes
     */
    unsigned getNTubes() const {return m_tubes.size();};

    /**
     * Get tube inner radius
     * @param i tube index
     * @return tube inner radius
     */
    double getTubeInnerR(unsigned i) const {return m_tubes.at(i).innerR / s_unit;}

    /**
     * Get tube outer radius
     * @param i tube index
     * @return tube outer radius
     */
    double getTubeOuterR(unsigned i) const {return m_tubes.at(i).outerR / s_unit;}

    /**
     * Get tube length
     * @param i tube index
     * @return tube length
     */
    double getTubeLength(unsigned i) const {return m_tubes.at(i).length / s_unit;}

    /**
     * Get tube Z position
     * @param i tube index
     * @return tube Z position (start point in ARICH frame)
     */
    double getTubeZPosition(unsigned i) const {return m_tubes.at(i).zPosition / s_unit;}

    /**
     * Get number of wedges to be placed
     * @return number of wedges
     */
    unsigned getNWedges() const {return m_wedges.size();};

    /**
     * Get type of i-th wedge
     * @param i wedge index
     * @return wedge type
     */
    int getWedgeType(unsigned i) const {return  m_wedges.at(i).type;}

    /**
     * Get radius at which i-th wedge is placed
     * @param i wedge index
     * @return wedge radius
     */
    double getWedgeR(unsigned i) const {return  m_wedges.at(i).r / s_unit;}

    /**
     * Get phi angle  at which i-th wedge is placed
     * @param i wedge index
     * @return wedge phi position
     */
    double getWedgePhi(unsigned i) const {return  m_wedges.at(i).phi;}

    /**
     * Get Z position of i-th wedge
     * @param i wedge index
     * @return wedge Z position
     */
    double getWedgeZ(unsigned i) const {return  m_wedges.at(i).z / s_unit;}

    /**
     * Get material of i-th tube
     * @param i tube index
     * @return material name
     */
    const std::string& getTubeMaterial(unsigned i) const {return m_tubes.at(i).material;}

    /**
     * Get material of i-th wedge
     * @param i wedge index
     * @return material name
     */
    const std::string& getWedgeMaterial(unsigned i) const {return m_wedges.at(i).material;}

    /**
     * Get name of i-th tube
     * @param i tube index
     * @return volume name
     */
    const std::string& getTubeName(unsigned i) const {return m_tubes.at(i).name;}

    /**
     * Get default material of support structures
     * @return default material name
     */
    const std::string& getMaterial() const {return m_material;}

    /**
     * Get parameters of wedge
     * @param i wedge type
     * @return vector of wedge parameters
     */
    const std::vector<double> getWedge(unsigned i) const {std::vector<double> pars; if (i > m_nWedgeType || i == 0) { B2ERROR("invalid wedge number"); return pars;}; for (auto par : m_wedgePar[i - 1]) pars.push_back(par / s_unit); return pars;}

    /**
     * Add box volume
     * @param name volume name
     * @param material volume material
     * @param size array of side sizes
     * @param position array of x,y,z position
     * @param rotation array of x,y,z rotations
     */
    void addBox(const std::string& name, const std::string& material, double size[3], double position[3], double rotation[3])
    {
      m_boxes.push_back({{size[0], size[1], size[2]}, {position[0], position[1], position[2]}, {rotation[0], rotation[1], rotation[2]}, name, material});
    }

    /**
     * Re-set parameters of existing box
     * @param i box index
     * @param name volume name
     * @param material volume material
     * @param size array of side sizes
     * @param position array of x,y,z position
     * @param rotation array of x,y,z rotations
     */
    void setBox(int i, const std::string& name, const std::string& material, double size[3], double position[3], double rotation[3])
    {
      m_boxes[i] = {{size[0], size[1], size[2]}, {position[0], position[1], position[2]}, {rotation[0], rotation[1], rotation[2]}, name, material};
    }

    /**
     * Get number of box volumes
     * @return number of box volumes
     */
    unsigned getNBoxes() const
    {
      return m_boxes.size();
    }

    /**
     * Get box paramaters
     * @param i box index
     * @return box parameters
     */
    box getBox(unsigned i) const
    {
      return m_boxes[i];
    }

    /**
     * Clear container of boxes
     */
    void clearBoxes()
    {
      m_boxes.clear();
    }



  private:

    std::vector<tube> m_tubes;           /**< vector of tube volumes to be placed */
    std::string m_material;              /**< default material of support structure */
    static const unsigned m_nWedgeType = 3; /**< number of wedge types */
    std::vector<double> m_wedgePar[m_nWedgeType];   /**< parameters of support wedges (joints) */
    std::vector<wedge> m_wedges;         /**< vector of wedge volumes to be placed */
    std::vector<box> m_boxes;            /**< vector of box volumes to be places */

    ClassDefOverride(ARICHGeoSupport, 1); /**< ClassDef */

  };

} // end namespace Belle2
