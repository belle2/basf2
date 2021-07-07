/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "analysis/OrcaKinFit/BaseConstraint.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace Belle2 {
  namespace OrcaKinFit {

    BaseConstraint::BaseConstraint()
      : name(nullptr)
    {
      setName("???");
    }

    BaseConstraint::BaseConstraint(const BaseConstraint& rhs)
      : name(nullptr)
    {
      if (rhs.name) setName(rhs.name);
      else setName("???");
    }
    BaseConstraint& BaseConstraint::operator= (const BaseConstraint& rhs)
    {
      if (this != &rhs) {
        if (rhs.name) setName(rhs.name);
        else setName("???");
      }
      return *this;
    }

    BaseConstraint::~BaseConstraint()
    {
      delete[] name;
    }

    const char* BaseConstraint::getName() const
    {
      return name;
    }

    void  BaseConstraint::setName(const char* name_)
    {
      if (name_ == nullptr) return;
      size_t l = strlen(name_);
      if (name) delete[] name;
      name = new char[l + 1];
      strcpy(name, name_);
    }

    double BaseConstraint::getError() const
    {
      assert(false);
      return 0;
    }

    std::ostream&  BaseConstraint::print(std::ostream& os) const
    {
      os << getName() << "=" << getValue();
      return os;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace
