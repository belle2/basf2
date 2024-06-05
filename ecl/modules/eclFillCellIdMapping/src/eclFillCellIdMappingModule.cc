/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclFillCellIdMapping/eclFillCellIdMappingModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLCellIdMapping.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* Basf2 headers. */
#include <framework/geometry/B2Vector3.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLFillCellIdMapping);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLFillCellIdMappingModule::ECLFillCellIdMappingModule() : Module()
{
  // Set module properties
  setDescription("Fills a dataobject that provides maping between cell id and store arrays and neighbour maps");
}

void ECLFillCellIdMappingModule::initialize()
{
  m_eclCalDigits.isRequired();
  m_eclCellIdMapping.registerInDataStore();

  // make neighbourmap
  m_NeighbourMap5 = new ECLNeighbours("N", 2, true); //sort them for ecl variable getters
  m_NeighbourMap7 = new ECLNeighbours("N", 3, true); //sort them for ecl variable getters
  m_NeighbourMap9  = new ECLNeighbours("N", 4, true); //sort them for ecl variable getters
  m_NeighbourMap11 = new ECLNeighbours("N", 5, true); //sort them for ecl variable getters

  // get phi, theta, phiid, thetaid values
  m_CellIdToPhi.resize(ECLElementNumbers::c_NCrystals + 1);
  m_CellIdToTheta.resize(ECLElementNumbers::c_NCrystals + 1);
  m_CellIdToPhiId.resize(ECLElementNumbers::c_NCrystals + 1);
  m_CellIdToThetaId.resize(ECLElementNumbers::c_NCrystals + 1);

  // Geometry instance.
  m_geom = ECLGeometryPar::Instance();

  for (int idx = 1; idx <= ECLElementNumbers::c_NCrystals; idx++) {
    B2Vector3D vectorPosition = m_geom->GetCrystalPos(idx - 1);

    m_CellIdToPhi[idx] = vectorPosition.Phi();
    m_CellIdToTheta[idx] = vectorPosition.Theta();

    m_geom->Mapping(idx - 1);
    m_CellIdToPhiId[idx] = m_geom->GetPhiID();
    m_CellIdToThetaId[idx] = m_geom->GetThetaID();
  }
}

void ECLFillCellIdMappingModule::event()
{
  if (!m_eclCellIdMapping) {
    m_eclCellIdMapping.create();

    for (int idx = 1; idx <= ECLElementNumbers::c_NCrystals; idx++) {
      m_eclCellIdMapping->setCellIdToNeighbour5(idx, m_NeighbourMap5->getNeighbours(idx));
      m_eclCellIdMapping->setCellIdToNeighbour7(idx, m_NeighbourMap7->getNeighbours(idx));
      m_eclCellIdMapping->setCellIdToNeighbour9(idx, m_NeighbourMap9->getNeighbours(idx));
      m_eclCellIdMapping->setCellIdToNeighbour11(idx, m_NeighbourMap11->getNeighbours(idx));
      m_eclCellIdMapping->setCellIdToPhi(idx, m_CellIdToPhi[idx]);
      m_eclCellIdMapping->setCellIdToTheta(idx, m_CellIdToTheta[idx]);
      m_eclCellIdMapping->setCellIdToPhiId(idx, m_CellIdToPhiId[idx]);
      m_eclCellIdMapping->setCellIdToThetaId(idx, m_CellIdToThetaId[idx]);
    }
  }

  // Fill a vector that can be used to map cellid -> store array position for eclCalDigits.
  m_eclCellIdMapping->reset();

  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_eclCellIdMapping->setCellIdToStoreArray(m_eclCalDigits[i]->getCellId(), i);
  }
}

void ECLFillCellIdMappingModule::terminate()
{
  if (m_NeighbourMap5) delete m_NeighbourMap5;
  if (m_NeighbourMap7) delete m_NeighbourMap7;
  if (m_NeighbourMap9) delete m_NeighbourMap9;
  if (m_NeighbourMap11) delete m_NeighbourMap11;
}
