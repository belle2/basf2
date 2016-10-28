#ifndef GEOMETRY_H
#define GEOMETRY_H

// This file is meant to contain several (relatively) short auxillary functions
// that depend on geometric properties of the calorimeter and specific
// parameters of COPPER.

// For the sake of clarity it may be prudent to rename these functions from "GetX" to "ChannelIdToX"

namespace Belle2 {
  int GetCrateId(int ch_num);
  // Identical to GetCrateId
  int GetCollectorId(int ch_num);
  int GetShaperId(int ch_num);
  int GetCrateChannelId(int ch_num);
  int GetShaperChannelId(int ch_num);

  int GetChannelCount();
}

#endif // GEOMETRY_H
