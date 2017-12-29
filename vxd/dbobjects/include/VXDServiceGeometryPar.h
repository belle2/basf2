/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {

  /**
  * The Class for VXD doc box
  */
  class VXDBoxTypesPar: public TObject {
  public:

    /**Constructor */
    VXDBoxTypesPar(const std::string& name = "", const std::string& material = "", int identifier = 0,
                   double width = 0, double length = 0, double height = 0) :
      m_name(name), m_material(material), m_identifier(identifier), m_width(width), m_length(length), m_height(height)
    {}
    //! Get width in mm
    double getWidth(void) const { return m_width; }
    //! Get length in mm
    double getLength(void) const { return m_length; }
    //! Get height in mm
    double getHeight(void) const { return m_height; }
    //! Get material
    std::string getMaterial(void) const { return m_material; }
    //! Get name
    std::string getName(void) const { return m_name; }
    //! Get identifier
    int getIdentifier(void) const { return m_identifier; }

  private:
    //! Name
    std::string m_name;
    //! Material
    std::string m_material;
    //! Identifier
    int m_identifier;
    //! Width in mm
    double m_width;
    //! Lenght in mm
    double m_length;
    //! Height in mm
    double m_height;

    ClassDef(VXDBoxTypesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for Slot types
  */
  class VXDSlotsPar: public TObject {
  public:

    /**Constructor */
    VXDSlotsPar(const std::string& type = ""): m_type(type) {}
    //! Get type
    const std::string& getType(void) const { return m_type; }
    //! Get slot numbers
    const std::vector<double>& getSlotNumbers(void) const { return m_slotNumbers; }
    //! Get slot numbers
    std::vector<double>& getSlotNumbers(void) { return m_slotNumbers; }

  private:
    //! Slots numbers
    std::vector<double> m_slotNumbers;
    //! Type
    std::string m_type;

    ClassDef(VXDSlotsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD doc box envelope
  */
  class VXDBoxPositionsPar: public TObject {
  public:

    /** Constructor */
    VXDBoxPositionsPar(const std::string& name = "", double minZ = 0, double maxZ = 0, double minR = 0, double maxR = 0,
                       int divisions = 0) :
      m_name(name), m_minZ(minZ), m_maxZ(maxZ), m_minR(minR), m_maxR(maxR), m_divisions(divisions)
    {}
    //! Get minZ in mm
    double getMinZ(void) const { return m_minZ; }
    //! Get maxZ in mm
    double getMaxZ(void) const { return m_maxZ; }
    //! Get minR in mm
    double getMinR(void) const { return m_minR; }
    //! Get maxR in mm
    double getMaxR(void) const { return m_maxR; }
    //! Get divisions
    int getDivisions(void) const { return m_divisions; }
    //! Get slots
    const std::vector<VXDSlotsPar>& getSlots(void) const { return m_slots; }
    //! Get slots
    std::vector<VXDSlotsPar>& getSlots(void) { return m_slots; }
    //! Get name
    const std::string& getName(void) const { return m_name; }

  private:
    //! Name
    std::string m_name;
    //! MinZ in mm
    double m_minZ;
    //! MaxZ in mm
    double m_maxZ;
    //! MinR in mm
    double m_minR;
    //! MaxR in mm
    double m_maxR;
    //! Divisions
    int m_divisions;
    //! Slots for doc boxes
    std::vector<VXDSlotsPar> m_slots;

    ClassDef(VXDBoxPositionsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD service geometry
  */
  class VXDServiceGeometryPar: public TObject {

  public:
    /** Constructor */
    VXDServiceGeometryPar(bool recordBackground = false): m_recordBackground(recordBackground) {}
    //! Record background
    bool getRecordBackground(void) const { return m_recordBackground; }
    //! Get boxes
    const std::vector<VXDBoxTypesPar>& getBoxTypes(void) const { return m_vxdBoxTypes; }
    //! Get boxes
    std::vector<VXDBoxTypesPar>& getBoxTypes(void)  { return m_vxdBoxTypes; }
    //! Get positions
    const std::vector<VXDBoxPositionsPar>& getPositions(void) const { return m_vxdBoxPositions; }
    //! Get positions
    std::vector<VXDBoxPositionsPar>& getPositions(void)  { return m_vxdBoxPositions; }

  private:
    //! Record background
    bool m_recordBackground;
    //! Vector with types of dock boxes
    std::vector<VXDBoxTypesPar> m_vxdBoxTypes;
    //! Vector with positions of dock boxes
    std::vector<VXDBoxPositionsPar> m_vxdBoxPositions;

    ClassDef(VXDServiceGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

