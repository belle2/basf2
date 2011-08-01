/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <geometry/utilities.h>

#include <boost/algorithm/string.hpp>

#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4VisAttributes.hh>


using namespace std;

namespace Belle2 {
  namespace geometry {
    G4Colour parseColor(string colorString)
    {
      boost::to_lower(colorString);
      double red(0), green(0), blue(0), alpha(0);
      if (colorString[0] == '#') {
        size_t size = colorString.size();
        unsigned int colorValue;
        stringstream in(colorString);
        in.get();
        in >> std::hex >> colorValue;
        if (in.fail()) size = 0;
        switch (size) {
          case 4: //#rgb, add alpha since none was specified
            colorValue = (colorValue << 4) + 15;
          case 5: //#rgba
            red   = ((colorValue & 0xf000) >> 12) / 15.;
            green = ((colorValue & 0x0f00) >>  8) / 15.;
            blue  = ((colorValue & 0x00f0) >>  4) / 15.;
            alpha = ((colorValue & 0x000f) >>  0) / 15.;
            break;
          case 7: //#rrggbb, add alpha since none was specified
            colorValue = (colorValue << 8) + 255;
          case 9: //#rrggbbaa
            red   = ((colorValue & 0xff000000) >> 24) / 255.;
            green = ((colorValue & 0x00ff0000) >> 16) / 255.;
            blue  = ((colorValue & 0x0000ff00) >>  8) / 255.;
            alpha = ((colorValue & 0x000000ff) >>  0) / 255.;
            break;
          default:
            B2WARNING("Could not parse color string '" + colorString + "'" << endl);
        }
      } else if (colorString.substr(0, 3) == "rgb") {
        //Parse value of the type rgb(1.0,1.0,1.0)
        size_t startPos = colorString.find("(");
        size_t stopPos = colorString.find(")");
        string ws(" \t\r\n,");
        stringstream in(colorString.substr(startPos + 1, stopPos - startPos - 1));
        in >> red;
        while (ws.find(in.peek()) != string::npos) in.get();
        in >> green;
        while (ws.find(in.peek()) != string::npos) in.get();
        in >> blue;
        while (ws.find(in.peek()) != string::npos) in.get();
        in >> alpha;
        red   = min(1.0, max(0.0, red));
        green = min(1.0, max(0.0, green));
        blue  = min(1.0, max(0.0, blue));
        alpha = min(1.0, max(0.0, alpha));
      }
      return G4Colour(red, green, blue, alpha);
    }

    void setColor(G4LogicalVolume &volume, const string &color)
    {
      G4VisAttributes* attr = const_cast<G4VisAttributes*>(volume.GetVisAttributes());
      if (!attr) attr = new G4VisAttributes();
      attr->SetColor(parseColor(color));
      volume.SetVisAttributes(attr);
    }
    void setVisibility(G4LogicalVolume &volume, bool visible)
    {
      G4VisAttributes* attr = const_cast<G4VisAttributes*>(volume.GetVisAttributes());
      if (!attr) attr = new G4VisAttributes();
      attr->SetVisibility(visible);
      volume.SetVisAttributes(attr);
    }
  }
} //Belle2 namespace
