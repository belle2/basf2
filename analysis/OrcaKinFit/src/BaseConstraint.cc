/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (ferber@physics.ubc.ca) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "analysis/OrcaKinFit/BaseConstraint.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace Belle2 {

  namespace OrcaKinFit {

    BaseConstraint::BaseConstraint()
      : name(0)
    {
      setName("???");
    }

    BaseConstraint::BaseConstraint(const BaseConstraint& rhs)
      : name(0)
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
      //B2INFO( "destroying BaseConstraint with name" << name);
      delete[] name;
    }

    const char* BaseConstraint::getName() const
    {
      return name;
    }

    void  BaseConstraint::setName(const char* name_)
    {
      if (name_ == 0) return;
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
