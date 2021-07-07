/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <display/EveTower.h>

#include <TEveCaloData.h>
#include <TEveSelection.h>
#include <TEveManager.h>


using namespace Belle2;

void EveTower::SelectElement(Bool_t state)
{
  auto& cellVector = m_caloData->GetCellsSelected();
  if (state)
    cellVector.emplace_back(m_id, 0); //tower id, slice 0
  else
    cellVector.clear();
  m_caloData->CellSelectionChanged();

  //to emulate a user click, also select the parent ECLData object
  //otherwise there will be no SelectionRepeated() signal if the
  //user clicks on another cluster after we're done here.
  //Note: also needs to be done without generating a selection event
  if (!gEve->GetSelection()->HasChild(m_caloData))
    gEve->GetSelection()->AddElement(m_caloData);

  //without selecting the visualisation objects, there is no highlighting, so lets do that, too.
  TEveElement::Set_t vizset;
  m_caloData->FillImpliedSelectedSet(vizset);
  for (TEveElement* viz : vizset)
    viz->SelectElement(state);
}
