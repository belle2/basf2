/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {
  /** Implements a colour palette, see http://sobac.com/sobac/tangocolors.htm
   *
   * Functions take a name (Aluminium, Slate, Chameleon, ...) and an ID
   * (1: light, 2: medium, 3: dark)
   * Invalid names/id produce an exception.
   */
  namespace TangoPalette {
    /** Get six-digit hex code (\#abcdef) for given name in tango colour palette.
     *
     * e.g. getHex("Aluminium", 3) returns "\#eeeeec".
     * */
    const char* getHex(const std::string& tangoName, int tangoId = 1);

    /** Get TColor ID for given name in tango colour palette.
     *
     * Use this for setting colours for ROOT objects. */
    int getTColorID(const std::string& tangoName, int tangoId = 1);
  }
}
