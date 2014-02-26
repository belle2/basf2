/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef BRANCHOF_H
#define BRANCHOF_H

#include <stddef.h>

#include "pp_pow.h"

#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/repetition/enum.hpp>


namespace Belle2 {
  namespace CDCLocalTracking {



    template<class Type, class Tag>
    class BranchOf {
    public:
      typedef Type value_type;
      typedef Tag tag;

      const value_type& getValue() const
      { return m_value; }

      void setValue(const value_type& value)
      {  m_value = value; }


    private:


      value_type m_value;

    };

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // STATICSTRING_H



