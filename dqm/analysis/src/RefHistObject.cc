/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "dqm/analysis/RefHistObject.h"

using namespace Belle2;

RefHistObject::~RefHistObject()
{
}

void RefHistObject::update(TH1* ref, TH1* refCopy, TCanvas* canvas)
{

  m_refHist = std::unique_ptr<TH1>(ref); //nullptr is okay here
  m_refCopy = std::unique_ptr<TH1>(refCopy);
  m_canvas  = std::unique_ptr<TCanvas>(canvas);
}

void RefHistObject::resetBeforeEvent(void)
{
  m_refHist = nullptr;
  m_refCopy = nullptr;
  m_canvas = nullptr;

  /*
  m_refHist.reset();
  m_refCopy.reset();
  m_canvas.reset();
  */
}
