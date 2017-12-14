// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
// Author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
// Author Loretta Negrini, Univ. Insubria <mailto:loryneg@gmail.com>
// Author Silvia Bonfanti, Univ. Insubria <mailto:silviafisica@gmail.com>
// Version $Id: EUTelMimosa18Detector.cc,v 1.8 2008-08-20 12:50:40 bulgheroni Exp $
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */

// personal includes ".h"
#include <eutel/eudaq/EUTELESCOPE.h>
#include <eutel/eudaq/EUTelMimosa18Detector.h>

// system includes <>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;
using namespace eutelescope;

EUTelMimosa18Detector::EUTelMimosa18Detector() : EUTelPixelDetector()
{

  _xMin = 0;
  _xMax = 511;

  _yMin = 0;
  _yMax = 511;

  _markerPos.clear();
  _markerPos.push_back(0);
  _markerPos.push_back(1);
  _markerPos.push_back(510);
  _markerPos.push_back(511);

  _signalPolarity = -1;

  _name = "Mimosa18";

  _xPitch = 0.01;
  _yPitch = 0.01;

  // now the matrix boundaries with markers
  EUTelROI ch0wm(0,  0, 255, 255);
  EUTelROI ch1wm(256,  0, 511, 255);
  EUTelROI ch2wm(0, 256, 255, 511);
  EUTelROI ch3wm(256, 256, 511, 511);

  _subChannelsWithMarkers.push_back(ch0wm);
  _subChannelsWithMarkers.push_back(ch1wm);
  _subChannelsWithMarkers.push_back(ch2wm);
  _subChannelsWithMarkers.push_back(ch3wm);

  // now the matrix boundaries without markers
  EUTelROI ch0wom(0,  0, 253, 255);
  EUTelROI ch1wom(254,  0, 507, 255);
  EUTelROI ch2wom(0, 256, 253, 511);
  EUTelROI ch3wom(254, 256, 507, 511);

  _subChannelsWithoutMarkers.push_back(ch0wom);
  _subChannelsWithoutMarkers.push_back(ch1wom);
  _subChannelsWithoutMarkers.push_back(ch2wom);
  _subChannelsWithoutMarkers.push_back(ch3wom);



}

bool EUTelMimosa18Detector::hasSubChannels() const
{
  if (_subChannelsWithoutMarkers.size() != 0) return true;
  else return false;
}

std::vector< EUTelROI > EUTelMimosa18Detector::getSubChannels(bool withMarker) const
{

  if (withMarker) return _subChannelsWithMarkers;
  else  return _subChannelsWithoutMarkers;

}

EUTelROI EUTelMimosa18Detector::getSubChannelBoundary(size_t iChan, bool withMarker) const
{
  if (withMarker) return _subChannelsWithMarkers.at(iChan);
  else return _subChannelsWithoutMarkers.at(iChan);

}


void EUTelMimosa18Detector::setMode(string mode)
{

  _mode = mode;

}

void EUTelMimosa18Detector::print(ostream& os) const
{

  size_t w = 35;

  string pol = "negative";
  if (_signalPolarity > 0) pol = "positive";

  os << resetiosflags(ios::right)
     << setiosflags(ios::left)
     << setfill('.') << setw(w) << setiosflags(ios::left) << "Detector name " << resetiosflags(ios::left) << " " << _name << endl
     << setw(w) << setiosflags(ios::left) << "Mode " << resetiosflags(ios::left) << " " << _mode << endl
     << setw(w) << setiosflags(ios::left) << "Pixel along x " << resetiosflags(ios::left) << " from " << _xMin << " to " << _xMax << endl
     << setw(w) << setiosflags(ios::left) << "Pixel along y " << resetiosflags(ios::left) << " from " << _yMin << " to " << _yMax << endl
     << setw(w) << setiosflags(ios::left) << "Pixel pitch along x " << resetiosflags(ios::left) << " " << _xPitch << "  mm  "  << endl
     << setw(w) << setiosflags(ios::left) << "Pixel pitch along y " << resetiosflags(ios::left) << " " << _yPitch << "  mm  "  << endl
     << setw(w) << setiosflags(ios::left) << "Signal polarity " << resetiosflags(ios::left) << " " << pol <<  setfill(' ') << endl;

  if (hasMarker()) {

    os << resetiosflags(ios::right) << setiosflags(ios::left);
    os << "Detector has the following columns (" << _markerPos.size() << ") used as markers: " << endl;

    vector< size_t >::const_iterator iter = _markerPos.begin();
    while (iter != _markerPos.end()) {

      os << "x = " << setw(15) << setiosflags(ios::right) << (*iter) << resetiosflags(ios::right) << endl;

      ++iter;
    }

  }
}
