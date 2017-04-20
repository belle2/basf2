/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  /** Directory structure for local lookup directories to allow for different
   * storage layouts: all payloads in one folder, all payloads in grouped in
   * folders by their name or grouped in folders by beginning of their
   * checksum
   */
  enum class EConditionsDirectoryStructure {
    /** directory contains all payloads directly */
    c_flatDirectory,
    /** directory contains all payloads as they are specified by payloadUrl
     * which might or might not include subdirectories */
    c_logicalSubdirectories,
    /** directories contains all payloads in subdirectories starting with the
     * hash values. i.e. if payload has checksum 0123456789ABCDEF it would be
     * stored in 0/12/$filename to evenly distribute all payloads across
     * subdirectories. This is the same scheme used for git objects
     */
    c_digestSubdirectories,
  };
}
