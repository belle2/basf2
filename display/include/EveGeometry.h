#pragma once

#include <Rtypes.h>

namespace Belle2 {
  /** Handles manipulation of detector geometry for the display. */
  namespace EveGeometry {
    /** Type of geometry shown. */
    enum EType {
      c_Full, /**< Full geometry converted from Geant4 (use this for non-standard Belle II setups!). */
      c_Simplified /**< a simplified Belle II geometry. */
    };
    /** Add TGeo geometry to Eve (only needs to be done once.) */
    void addGeometry(EType visMode);

    /** switch to given visualisation mode. */
    void setVisualisationMode(EType visMode);

    /** enable/disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void enableVolume(const char* name, bool only_daughters = false, bool enable = true);

    /** disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void disableVolume(const char* name, bool only_daughters = false);

    /** set fill color of the volume 'name' to 'col'. */
    void setVolumeColor(const char* name, Color_t col);

    /** Recursively set transparency of geometry (0: opaque, 100: fully transparent). */
    void setTransparency(int percent);

    /** find a point that is inside the top node. */
    double getMaxR();

    /** Save a geometry extract from the current state of the TGeo geometry.
     *
     * To actually create a reasonably sized extract, one needs to delete
     * all objects in Top_1 that one doesn't want to save. Selecting them
     * via their name and deleting them in a function should work, but really
     * just crashes. This is all quite horrible, really.
     */
    void saveExtract();

    /** Set custom path to the geometry extract (to change originally hard-coded value) */
    void setCustomExtractPath(const std::string& extractPath);

    /** Set custom path to the geometry extract with corrected TOP bars (to change originally hard-coded value) */
    void setCustomExtractPathTop(const std::string& extractPathTop);
  }
}

