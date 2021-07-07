/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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