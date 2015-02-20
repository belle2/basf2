#pragma once

#include <Rtypes.h>

namespace Belle2 {
  /** Handles manipulation of detector geometry for the display. */
  namespace EveGeometry {

    /** Add TGeo geometry to Eve (only needs to be done once.) */
    void addGeometry(bool fullgeo);

    /** enable/disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void enableVolume(const char* name, bool only_daughters = false, bool enable = true);

    /** disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void disableVolume(const char* name, bool only_daughters = false);

    /** set fill color of the volume 'name' to 'col'. */
    void setVolumeColor(const char* name, Color_t col);

    //find a point that is inside the top node
    double getMaxR();

    /** Save a geometry extract from the current state of the TGeo geometry. */
    void saveExtract();
  }
}

