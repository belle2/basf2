/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <map>
#include <vector>
#include <string>

namespace Belle2 {
  /** Internal class to store string -> index maps for extra info stored in Particle.
   *
   * An object of this class is used to store all strings used to identify the values added using
   * Particle::addExtraInfo() and keeps Particles fairly light (only values and a map ID are stored directly).
   * The necessary indirection is handled automatically by Particle::getExtraInfo().
   *
   * Modules registering a StoreArray<Particle> should always register a StoreObjPtr<ParticleExtraInfoMap>
   * to allow storing additional information.
   */
  class ParticleExtraInfoMap : public TObject {
  public:
    /** string -> index map. */
    typedef std::map<std::string, unsigned int> IndexMap;

    ParticleExtraInfoMap(): TObject() { }

    /** Return reference to map with given ID. */
    const IndexMap& getMap(unsigned int mapID) const { return m_maps[mapID]; }

    /** Find index for name in the given map, or return 0 if not found. */
    unsigned int getIndex(unsigned int mapID, const std::string& name) const;

    /** Return map ID to a map that has 'name' as first entry.
     *
     * Creates a new map if necessary.
     */
    unsigned int getMapForNewVar(const std::string& name);

    /** Return map ID to a map that has 'name' at place 'insertIndex' and is compatible with previous map (in oldMapID)
     *
     * Will reuse old map or another existing map if compatible, creates a new map if necessary.
     */
    unsigned int getMapForNewVar(const std::string& name, unsigned int oldMapID, unsigned int insertIndex);

    /** How many maps do we use? */
    unsigned int getNMaps() const { return m_maps.size(); }

  private:
    /** check if all entries in 'oldMap' prior to insertIndex are found in 'map' (with same idx). */
    static bool isCompatible(const IndexMap& oldMap, const IndexMap& map, unsigned int insertIndex);

    std::vector<IndexMap> m_maps; /**< List of string -> index maps. */

    ClassDef(ParticleExtraInfoMap, 1); /**< Internal class to store string -> index maps for user-defined variables in Particle. */
  };
}
