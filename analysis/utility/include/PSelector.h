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


namespace Belle2 {

  class Particle;

  /**
   *
   */
  struct SelectionRange {
    double xmin;
    double xmax;
    bool lmin;
    bool lmax;
    std::string varName;
    SelectionRange():
      xmin(0.0), xmax(0.0), lmin(false), lmax(false), varName("")
    {}
    SelectionRange(std::string varname):
      xmin(0.0), xmax(0.0), lmin(false), lmax(false), varName(varname)
    {}
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

  typedef std::vector<SelectionRange> SelRangeVector;
  typedef std::pair<std::string, double(*)(const Particle*)> FunctionDescr;

  struct SelectionCriteria {
    double(*function)(const Particle*);
    std::vector<unsigned int> childIndex;
    SelRangeVector ranges;
    SelectionCriteria(): function(0)
    {}
    SelectionCriteria(double(*fun)(const Particle*),
                      std::vector<unsigned int> childInd,
                      SelRangeVector selRanges):
      function(fun), childIndex(childInd), ranges(selRanges)
    {}
  };

  /**
   *
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


    void addFunction(std::string varName, std::string varDescription,
                     double(*)(const Particle*));

    bool addSelection(std::string cuts);

    bool select(const Particle* particle);

    void listVariables() const;

    void listCuts() const;

    void listCuts(std::string& str) const;


  private:

    /**
     * Initialize functionList
     */
    void initialize();

    /**
     *
     */
    bool appendRange(std::string varname, std::string cuts, SelRangeVector& Ranges);

    /**
     *
     */
    bool getRange(std::string cut, SelectionRange& range);

    /**
     *
     */
    bool selectLevel0(double x, const SelRangeVector& ranges);

    /**
     *
     */
    bool intheRange(double x, const SelectionRange& range);

    std::vector<SelectionCriteria> m_selectionList; /**< selection criteria */
    static std::map<std::string, FunctionDescr>  m_functionList; /**< functions */


  };


} // Belle2 namespace

#endif


