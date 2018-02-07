// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
// Author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
// Version $Id: EUTelMimosa26Detector.cc,v 1.1 2009-07-31 09:45:49 bulgheroni Exp $
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
#include <eutel/eudaq/EUTelMimosa26Detector.h>

// system includes <>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;
using namespace eutelescope;

EUTelMimosa26Detector::EUTelMimosa26Detector() : EUTelPixelDetector()
{

  _xMin = 0;
  _xMax = 1151;

  _yMin = 0;
  _yMax = 575;

  _signalPolarity = 1;

  _name = "Mimosa26";

  _xPitch = 0.018;
  _yPitch = 0.018;

  // prepare and add the subchannels starting from the one with
  // markers in it
  EUTelROI ch0wm(0,  0, 287, 575);
  EUTelROI ch1wm(288,  0, 575, 575);
  EUTelROI ch2wm(576,  0, 863, 575);
  EUTelROI ch3wm(864,  0, 1151, 575);

  _subChannelsWithMarkers.push_back(ch0wm);
  _subChannelsWithMarkers.push_back(ch1wm);
  _subChannelsWithMarkers.push_back(ch2wm);
  _subChannelsWithMarkers.push_back(ch3wm);

  _subChannelsWithoutMarkers.push_back(ch0wm);
  _subChannelsWithoutMarkers.push_back(ch1wm);
  _subChannelsWithoutMarkers.push_back(ch2wm);
  _subChannelsWithoutMarkers.push_back(ch3wm);


}

void EUTelMimosa26Detector::setMode(string mode)
{

  _mode = mode;

}

bool EUTelMimosa26Detector::hasSubChannels() const
{

  if (_subChannelsWithoutMarkers.size() != 0) return true;
  else return false;

}

std::vector< EUTelROI > EUTelMimosa26Detector::getSubChannels(bool withMarker) const
{

  if (withMarker) return _subChannelsWithMarkers;
  else  return _subChannelsWithoutMarkers;

}

EUTelROI EUTelMimosa26Detector::getSubChannelBoundary(size_t iChan, bool withMarker) const
{
  if (withMarker) return _subChannelsWithMarkers.at(iChan);
  else return _subChannelsWithoutMarkers.at(iChan);

}

void EUTelMimosa26Detector::print(ostream& os) const
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
