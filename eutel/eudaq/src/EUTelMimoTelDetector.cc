// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
// Author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
// Author Loretta Negrini, Univ. Insubria <mailto:loryneg@gmail.com>
// Author Silvia Bonfanti, Univ. Insubria <mailto:silviafisica@gmail.com>
// Version $Id: EUTelMimoTelDetector.cc,v 1.5 2008-08-20 12:50:40 bulgheroni Exp $
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
#include <eutel/eudaq/EUTelMimoTelDetector.h>

// system includes <>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;
using namespace eutelescope;

EUTelMimoTelDetector::EUTelMimoTelDetector() : EUTelPixelDetector()
{

  _xMin = 0;
  _xMax = 263;

  _yMin = 0;
  _yMax = 255;

  _markerPos.clear();
  _markerPos.push_back(0);
  _markerPos.push_back(1);
  _markerPos.push_back(66);
  _markerPos.push_back(67);
  _markerPos.push_back(132);
  _markerPos.push_back(133);
  _markerPos.push_back(198);
  _markerPos.push_back(199);

  _signalPolarity = -1;

  _name = "MimoTel";

  _xPitch = 0.03;
  _yPitch = 0.03;

  // prepare and add the subchannels starting from the one with
  // markers in it
  EUTelROI ch0wm(0,  0,  65, 255);
  EUTelROI ch1wm(66,  0, 131, 255);
  EUTelROI ch2wm(132,  0, 197, 255);
  EUTelROI ch3wm(198,  0, 263, 255);

  _subChannelsWithMarkers.push_back(ch0wm);
  _subChannelsWithMarkers.push_back(ch1wm);
  _subChannelsWithMarkers.push_back(ch2wm);
  _subChannelsWithMarkers.push_back(ch3wm);

  // now the one without markers
  EUTelROI ch0wom(0,   0,  63,  255);
  EUTelROI ch1wom(64,   0, 127,  255);
  EUTelROI ch2wom(128,   0, 191,  255);
  EUTelROI ch3wom(192,   0, 255,  255);

  _subChannelsWithoutMarkers.push_back(ch0wom);
  _subChannelsWithoutMarkers.push_back(ch1wom);
  _subChannelsWithoutMarkers.push_back(ch2wom);
  _subChannelsWithoutMarkers.push_back(ch3wom);


}

void EUTelMimoTelDetector::setMode(string mode)
{

  _mode = mode;

}

bool EUTelMimoTelDetector::hasSubChannels() const
{

  if (_subChannelsWithoutMarkers.size() != 0) return true;
  else return false;

}

std::vector< EUTelROI > EUTelMimoTelDetector::getSubChannels(bool withMarker) const
{

  if (withMarker) return _subChannelsWithMarkers;
  else  return _subChannelsWithoutMarkers;

}

EUTelROI EUTelMimoTelDetector::getSubChannelBoundary(size_t iChan, bool withMarker) const
{
  if (withMarker) return _subChannelsWithMarkers.at(iChan);
  else return _subChannelsWithoutMarkers.at(iChan);

}

void EUTelMimoTelDetector::print(ostream& os) const
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
    os << "Detector has the following columns (" << _markerPos.size() << ")  used as markers: " << endl;

    vector< size_t >::const_iterator iter = _markerPos.begin();
    while (iter != _markerPos.end()) {

      os << "x = " << setw(15) << setiosflags(ios::right) << (*iter) << resetiosflags(ios::right) << endl;

      ++iter;
    }

  }
}
