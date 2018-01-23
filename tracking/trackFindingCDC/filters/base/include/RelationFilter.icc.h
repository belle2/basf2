/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template<class AObject>
    RelationFilter<AObject>::RelationFilter() = default;

    template<class AObject>
    RelationFilter<AObject>::~RelationFilter() = default;

    template<class AObject>
    std::vector<AObject*> RelationFilter<AObject>::getPossibleTos(
      AObject* from __attribute__((unused)),
      const std::vector<AObject*>& objects) const
    {
      return objects;
    }

    template<class AObject>
    Weight RelationFilter<AObject>::operator()(const AObject& from __attribute__((unused)),
                                               const AObject& to __attribute__((unused)))
    {
      return 1;
    }

    template<class AObject>
    Weight RelationFilter<AObject>::operator()(const Relation<AObject>& relation)
    {
      const AObject* from = relation.getFrom();
      const AObject* to = relation.getTo();

      if (from == to) return NAN; // Prevent relation to same.
      if ((from == nullptr) or (to == nullptr)) return NAN;
      return operator()(*from, *to);
    }
  }
}