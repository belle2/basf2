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

  class GearDir;


  /**
  * The Class for VXD doc box
  */

  class VXDBoxTypesPar: public TObject {
  public:

    //! Default constructor
    VXDBoxTypesPar() {}
    //! Constructor using Gearbox
    explicit VXDBoxTypesPar(const GearDir& content) { read(content); }
    //! Read parameters from Gearbox
    void read(const GearDir&);
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
    //! Width in mm
    double m_width;
    //! Lenght in mm
    double m_length;
    //! Height in mm
    double m_height;
    //! Material
    std::string m_material;
    //! Name
    std::string m_name;
    //! Identifier
    int m_identifier;

    ClassDef(VXDBoxTypesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for Slot types
  */

  class VXDSlotTypesPar: public TObject {
  public:

    //! Default constructor
    VXDSlotTypesPar() {}
    //! Constructor using Gearbox
    explicit VXDSlotTypesPar(const GearDir& content) { read(content); }
    //! Read parameters from Gearbox
    void read(const GearDir&);
    //! Get type
    std::string getType(void) const { return m_type; }
    //! Get slot numbers
    std::vector<double> getSlots(void) const { return m_slots; }

  private:
    //! Slots numbers
    std::vector<double> m_slots;
    //! Type
    std::string m_type;

    ClassDef(VXDSlotTypesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD doc box envelope
  */

  class VXDBoxPositionsPar: public TObject {
  public:

    //! Default constructor
    VXDBoxPositionsPar() {}
    //! Constructor using Gearbox
    explicit VXDBoxPositionsPar(const GearDir& content) { read(content); }
    //! Read parameters from Gearbox
    void read(const GearDir&);
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
    std::vector<VXDSlotTypesPar> getSlotTypes(void) const { return m_slotTypes; }
    //! Get name
    std::string getName(void) const { return m_name; }

  private:
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
    std::vector<VXDSlotTypesPar> m_slotTypes;
    //! Name
    std::string m_name;

    ClassDef(VXDBoxPositionsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD service geometry
  */

  class VXDServiceGeometryPar: public TObject {

  public:

    //! Default constructor
    VXDServiceGeometryPar() {}

    //! Constructor using Gearbox
    explicit VXDServiceGeometryPar(const GearDir& content) { read(content); }

    //! Destructor
    ~VXDServiceGeometryPar() {}

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    //! Record background
    bool getRecordBackground(void) const { return m_recordBackground; }

    //! Get boxes
    std::vector<VXDBoxTypesPar> getBoxTypes(void) const { return m_vxdBoxTypes; }

    //! Get positions
    std::vector<VXDBoxPositionsPar> getPositions(void) const { return m_vxdBoxPositions; }

  private:

    //! Record background
    bool m_recordBackground;

    std::vector<VXDBoxTypesPar> m_vxdBoxTypes;

    std::vector<VXDBoxPositionsPar> m_vxdBoxPositions;

    ClassDef(VXDServiceGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

