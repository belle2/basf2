/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <generators/evtgen/EvtGenUtilities.h>

/* Basf2 headers. */
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

bool Belle2::generators::checkEvtGenDecayFile(
  const std::string& decayFile)
{
  const std::string defaultDecFile =
    FileSystem::findFile("decfiles/dec/DECAY_BELLE2.DEC", true);
  if (decayFile.empty()) {
    B2ERROR("No global decay file defined, please make sure "
            "the decay-file parameter is set correctly.");
    return false;
  }
  if (defaultDecFile.empty()) {
    B2WARNING("Cannot find default decay file.");
  } else if (defaultDecFile != decayFile) {
    B2INFO("Using non-standard decay file \"" << decayFile << "\".");
  }
  return true;
}
