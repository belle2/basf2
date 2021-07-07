/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    ECL::ECLGeometryPar* m_geom{nullptr};

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Neighbour maps */
    ECL::ECLNeighbours* m_NeighbourMap5{nullptr}; /**< 5x5 */
    ECL::ECLNeighbours* m_NeighbourMap7{nullptr}; /**< 7x7 */

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

