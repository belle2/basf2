/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/EventDataPlotter.h"

#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


EventDataPlotter::EventDataPlotter() :
  m_ptrPrimitivePlotter(new SVGPrimitivePlotter())
{
}

EventDataPlotter::EventDataPlotter(PrimitivePlotter* ptrPrimitivePlotter) :
  m_ptrPrimitivePlotter(ptrPrimitivePlotter)
{
  if (not m_ptrPrimitivePlotter) {
    B2WARNING("EventDataPlotter initialized with nullptr. Using default backend SVGPrimitivePlotter.");
    m_ptrPrimitivePlotter = new SVGPrimitivePlotter();
  }
}

EventDataPlotter::EventDataPlotter(const EventDataPlotter& eventDataPlotter) :
  m_ptrPrimitivePlotter(eventDataPlotter.m_ptrPrimitivePlotter ? eventDataPlotter.m_ptrPrimitivePlotter->clone() : nullptr)
{
}

EventDataPlotter::~EventDataPlotter()
{
  delete m_ptrPrimitivePlotter;
  m_ptrPrimitivePlotter = nullptr;
}

EventDataPlotter* EventDataPlotter::clone()
{
  return new EventDataPlotter(*this);
}

const std::string EventDataPlotter::save(const std::string& fileName)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.save(fileName);
  } else {
    return "";
  }
}



void EventDataPlotter::clear()
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.clear();
  }
}

float EventDataPlotter::getCanvasWidth()
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getCanvasWidth();
  } else {
    return NAN;
  }
}


float EventDataPlotter::getCanvasHeight()
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getCanvasHeight();
  } else {
    return NAN;
  }
}

void EventDataPlotter::setCanvasWidth(const float& width)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasWidth(width);
  }
}

void EventDataPlotter::setCanvasHeight(const float& height)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasHeight(height);
  }

}

