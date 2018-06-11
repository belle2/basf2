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
#include <map>
#include <vector>


namespace Belle2 {

  /**
  * The Class for VXD half shell payload
  */

  class VXDHalfShellPar: public TObject {

  public:
    /** Constructor */
    VXDHalfShellPar(std::string const& name = "", double shellAngle = 0) : m_name(name), m_shellAngle(shellAngle) {}
    /** get shell name */
    std::string getName() const { return m_name; }
    /** get shellAngle */
    double getShellAngle() const { return m_shellAngle; }
    /** add ladder */
    void addLadder(int layerID, int ladderID, double phi) { m_layers[layerID].push_back(std::pair<int, double>(ladderID, phi)); }
    /** get layers */
    const std::map< int, std::vector<std::pair<int, double>> >& getLayers() const { return m_layers; }

  private:
    /** Name of half shell */
    std::string m_name;
    /** Rotation angle of half shell */
    double m_shellAngle;
    /** Map for keeping ladderID and its phi rotation angle for all layers */
    std::map< int, std::vector<std::pair<int, double>> > m_layers;

    ClassDef(VXDHalfShellPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

