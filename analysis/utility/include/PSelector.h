/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PSELECTOR_H
#define PSELECTOR_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include <analysis/VariableManager/Manager.h>

namespace Belle2 {

  class Particle;

  /**
   * Structure to hold a selection range (an interval [xmin,xmax]) for a variable
   */
  struct SelectionRange {
    double xmin; /**< lower edge */
    double xmax; /**< upper edge */
    bool lmin;   /**< if false lower edge assumed to be -inf */
    bool lmax;   /**< if false upper edge assumed to be +inf */
    std::string varName;  /**< variable name */

    /**
     * Default constructor
     */
    SelectionRange():
      xmin(0.0), xmax(0.0), lmin(false), lmax(false), varName("")
    {}

    /**
     * Constructor using variable name
     * @param varname variable name
     */
    SelectionRange(std::string varname):
      xmin(0.0), xmax(0.0), lmin(false), lmax(false), varName(varname)
    {}

    /**
     * Print a selection range to std output
     */
    void print() const {
      if (lmin && lmax) {
        if (xmin != xmax) {
          std::cout << xmin << " <= " << varName << " <= " << xmax;
        } else {
          std::cout << varName << " == " << xmax;
        }
        return;
      }
      if (lmin) {std::cout << varName << " >= " << xmin; return;}
      if (lmax) {std::cout << varName << " <= " << xmax; return;}
    }

    /**
     * Print a selection range into stringstream
     * @param ss a reference to stringstream
     */
    void print(std::stringstream& ss) const {
      if (lmin && lmax) {
        if (xmin != xmax) {
          ss << xmin << " <= " << varName << " <= " << xmax;
        } else {
          ss << varName << " == " << xmax;
        }
        return;
      }
      if (lmin) {ss << varName << " >= " << xmin; return;}
      if (lmax) {ss << varName << " <= " << xmax; return;}
    }
  };

  /**
   * type definition of a selection range vector
   */
  typedef std::vector<SelectionRange> SelRangeVector;


  /**
   * Structure to hold selection criteria
   */
  struct SelectionCriteria {
    Variable::Manager::FunctionPtr function;   /**< function that returns the variable value */
    std::vector<unsigned int> childIndex; /**< a child tree indices */
    SelRangeVector ranges;                /**< vector of selection ranges */

    /**
     * Default constructor
     */
    SelectionCriteria(): function(0)
    {}

    /**
     * Full constructor
     * @param fun a pionter to function that returns the variable value
     * @param childInd a vector of child tree indices
     * @param selRanges a vector of selection ranges
     */
    SelectionCriteria(Variable::Manager::FunctionPtr fun,
                      std::vector<unsigned int> childInd,
                      SelRangeVector selRanges):
      function(fun), childIndex(childInd), ranges(selRanges)
    {}
  };

  /**
   * Class for Particle selection
   */
  class PSelector {

  public:

    /**
     * Constructor
     */
    PSelector();

    /**
     * Destructor
     */
    ~PSelector();


    /**
     * add selection criteria for a single variable
     * @param cuts selection criteria
     * @return true on success (variable exists, no syntax errors)
     */
    bool addSelection(std::string cuts);

    /**
     * select particle
     * @param particle a pointer to Particle
     * @return true if selected
     */
    bool select(const Particle* particle);

    /**
     * print defined variables to std output
     */
    void listVariables() const;

    /**
     * print selection criteria to std output
     */
    void listCuts() const;

    /**
     * write selection criteria to std::string
     * @param str a reference to string
     */
    void listCuts(std::string& str) const;


  private:

    /**
     * Append range to vector of ranges (called by addSelection)
     * @param varname a variable name
     * @param cut a string to parse
     * @param Ranges a reference to vector of ranges
     * @return true on success
     */
    bool appendRange(std::string varname, std::string cut, SelRangeVector& Ranges);

    /**
     * Get range from a string (called by appendRange)
     * @param cut a string to parse
     * @param range a reference to selection range
     * @return true on success
     */
    bool getRange(std::string cut, SelectionRange& range);

    /**
     * Returns true if x within ranges (called by select)
     * @param x selection variable value
     * @param ranges a vector of ranges
     * @return true if x within ranges
     */
    bool selectLevel0(double x, const SelRangeVector& ranges);

    /**
     * Returns true if x within range (called by selectLevel0)
     * @param x selection variable value
     * @param range the range
     * @return true if x within the range
     */
    bool intheRange(double x, const SelectionRange& range);

    std::vector<SelectionCriteria> m_selectionList; /**< selection criteria */

  };


} // Belle2 namespace

#endif


