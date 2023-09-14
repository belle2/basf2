/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Math/Vector3D.h>
#include <string>
#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {

  /** Class for saving raw hit data of the IR.  */
  class IRSimHit : public SimHitBase {
  public:

    /** Empty constructor for ROOT IO. */
    IRSimHit():
      m_posIn(0, 0, 0),
      m_momIn(0, 0, 0),
      m_posOut(0, 0, 0),
      m_momOut(0, 0, 0),
      m_PDGcode(0),
      m_depEnergy(0),
      m_Volname("noname") {}

    /** Full Constructor. */
    IRSimHit(
      ROOT::Math::XYZVector posIn,
      ROOT::Math::XYZVector momIn,
      ROOT::Math::XYZVector posOut,
      ROOT::Math::XYZVector momOut,
      int PDGcode,
      float depEnergy,
      const std::string& Volname
    ) :
      m_posIn(posIn),
      m_momIn(momIn),
      m_posOut(posOut),
      m_momOut(momOut),
      m_PDGcode(PDGcode),
      m_depEnergy(depEnergy),
      m_Volname(Volname) {}

    ~IRSimHit();

    /** The method to set.*/
    void setposIn(ROOT::Math::XYZVector posIn); /** Set position at start point*/
    void setmomIn(ROOT::Math::XYZVector momIn); /** Set momentum at start point */
    void setposOut(ROOT::Math::XYZVector posOut); /** Set position at end point */
    void setmomOut(ROOT::Math::XYZVector momOut); /** Set momentum at end point */
    void setPDGcode(int PDGcode);    /** Set PDF code */
    void setdepEnergy(float depEnergy); /** Set deposited energy */
    void setVolname(const std::string& Volname); /** Set volume name */

    /** The method to get.*/
    ROOT::Math::XYZVector getposIn();  /** return position at start point*/
    ROOT::Math::XYZVector getmomIn();  /** return momentum at start point*/
    ROOT::Math::XYZVector getposOut(); /** return position at end point*/
    ROOT::Math::XYZVector getmomOut(); /** return momentum at end point*/
    int getPDGcode();     /** return PDG code */
    float getdepEnergy(); /** return deposited energy */
    std::string getVolname(); /** return volume name */

  protected:

  private:

    ROOT::Math::XYZVector m_posIn;  /**< Start point of energy deposition in local coordinates. */
    ROOT::Math::XYZVector m_momIn;  /**< Momentum of particle at start of energy deposition. */
    ROOT::Math::XYZVector m_posOut; /**< End point of energy deposition in local coordinates. */
    ROOT::Math::XYZVector m_momOut; /**< Momentum of particle at end of energy deposition. */
    int m_PDGcode;     /**< PDG code of the particle producing hit. */
    float m_depEnergy; /**< Deposited energy */
    std::string m_Volname; /**< Volume name */

    /** ROOT Macro.*/
    ClassDef(IRSimHit, 2);

  };

} // end namespace Belle2
