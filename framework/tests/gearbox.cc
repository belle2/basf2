/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <gtest/gtest.h>
#include <string>
#include <queue>

using namespace std;
using namespace Belle2;

namespace {

  /** Test opening xml file and accessing nodes from Gearbox */
  TEST(GearBox, Open)
  {
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.emplace_back("file:");
    gb.setBackends(backends);
    gb.open("geometry/SuperKEKB.xml");
    queue<GearDir> nodes;
    nodes.push(GearDir());
    while (!nodes.empty()) {
      GearDir node = nodes.front();
      nodes.pop();
      for (const GearDir& child : node.getNodes("*")) {
        nodes.push(child);
      }
    }
    gb.close();
  }
}  // namespace
