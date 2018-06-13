/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <utility>
#include <map>
#include <TObject.h>
#include <TClonesArray.h>
#include <TClass.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>



namespace Belle2 {


  /**
   * @brief Base for calibration or alignment parameters identified by two unsigned shorts
   * Typically first is some identifier of a sub-detector element (sensor, layer, wire, board...)
   * and the second is number of parameter. Only the dublet has to be unique, so you can spare some
   * parameters or give same number to parameters with similar meaning. E.g. shift in local sensor U
   * system in VXD can have the same number as shift in layer local U system.
   *
   * Basically a representation of table with 3 columns, primary key = (id, param)
   * - (unsigned short) id    ... identifies element
   * - (unsigned short) param ... identifies element parameter
   * - (double)         value ... stored parameter value
   *
   */
  class DBObjCalibrationConstMapBase: public TObject {

  public:

    /// The underlying element type (also for param id)
    typedef int baseType;
    /// (element, parameter)
    typedef std::pair<baseType, baseType> ElementParameter;
    /// Map of (element, parameter) and its values
    typedef std::map<ElementParameter, double> ElementParameterMap;

    /// Get the underlying map of constants
    const ElementParameterMap& getMap() {return m_constants;}

    /// Constructor
    DBObjCalibrationConstMapBase() {}

    /// Destructor
    virtual ~DBObjCalibrationConstMapBase();

    /// Get parameter value for given element and parameter number
    virtual double get(baseType id, baseType param) const;

    /// Set parameter correction for given WireID and parameter number
    virtual void set(baseType id, baseType param, double value);;

    /// Add correction to already stored (or to 0. if not set yet) constant value (optionaly with minus sign)
    virtual double add(baseType id, baseType param, double value, bool subtractInsteadOfAdd = false);

    /// Write all elements' parameters' values to standard output
    virtual void dump() const;
  private:
    ElementParameterMap m_constants; /**< Map of map of all calibration elements' constants */

    ClassDef(DBObjCalibrationConstMapBase, 1); /**< Storage for alignment and calibration constants */

  };

} // end namespace Belle2

