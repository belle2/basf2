/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    /** add staggered ladder */
    void addLadderStaggered(int layerID, int ladderID, double phi, double shiftR, double shiftZ) { m_layersStaggered[layerID].push_back(std::tuple<int, double, double, double>(ladderID, phi, shiftR, shiftZ)); }
    /** get layers */
    const std::map< int, std::vector<std::pair<int, double>> >& getLayers() const { return m_layers; }
    /** get staggered layers */
    const std::map< int, std::vector<std::tuple<int, double, double, double>> >& getLayersStaggered() const { return m_layersStaggered; }

  private:
    /** Name of half shell */
    std::string m_name;
    /** Rotation angle of half shell */
    double m_shellAngle;
    /** Map for keeping ladderID and its phi rotation angle for all layers */
    std::map< int, std::vector<std::pair<int, double>> > m_layers;
    /** Map for keeping ladderID, its phi rotation angle, its radius shift and its shiftZ for all staggered layers */
    std::map< int, std::vector<std::tuple<int, double, double, double>> > m_layersStaggered;

    ClassDef(VXDHalfShellPar, 6);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

