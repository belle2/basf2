/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber (torben.ferber@desy.de)                    *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ECLCellIdMapping;
  class ECLCalDigit;

  namespace ECL {
    class ECLNeighbours;
    class ECLGeometryPar;
  }


  /**
  * Fills a dataobject that provides maping between cell id and ECLCalDigit store array
  *
  */
  class ECLFillCellIdMappingModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    ECLFillCellIdMappingModule();

    /** Initialize */
    virtual void initialize() override;

    /** Event */
    virtual void event() override;

    /** Terminate */
    virtual void terminate() override;


  private:
    /** Geometry */
    ECL::ECLGeometryPar* m_geom;

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Neighbour maps */
    ECL::ECLNeighbours* m_NeighbourMap5; /**< 5x5 */
    ECL::ECLNeighbours* m_NeighbourMap7; /**< 7x7 */

    /** Store object pointer: ECLCellIdToECLCalDigitMapping. */
    StoreObjPtr<ECLCellIdMapping> m_eclCellIdMapping;

    /** vector (8736+1 entries) with cell id to phi values  */
    std::vector<double> m_CellIdToPhi;

    /** vector (8736+1 entries) with cell id to theta values  */
    std::vector<double> m_CellIdToTheta;

    /** vector (8736+1 entries) with cell id to phi id values  */
    std::vector<int> m_CellIdToPhiId;

    /** vector (8736+1 entries) with cell id to theta id values  */
    std::vector<int> m_CellIdToThetaId;


  };
}

