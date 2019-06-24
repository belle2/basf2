/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * right(C) 2018 - Belle II Collaboration                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jo-Frederik Krohn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

namespace TreeFitter {

  /** constainer */
  class ConstraintConfiguration {
  public:

    /** constructor */
    ConstraintConfiguration() :
      m_massConstraintType(false),
      m_massConstraintListPDG( {}),
                             m_fixedToMotherVertexListPDG({}),
                             m_removeConstraintList({}),
                             m_automatic_vertex_constraining(false)
    {};

    /** constructor */
    ConstraintConfiguration(const bool& massConstraintType,
                            const std::vector<int>& massConstraintListPDG,
                            const std::vector<int>& fixedToMotherVertexListPDG,
                            const std::vector<int>& geoConstraintListPDG,
                            const std::vector<std::string>& removeConstraintList,
                            const bool& automatic_vertex_constraining) :
      m_massConstraintType(massConstraintType),
      m_massConstraintListPDG(massConstraintListPDG),
      m_fixedToMotherVertexListPDG(fixedToMotherVertexListPDG),
      m_geoConstraintListPDG(geoConstraintListPDG),
      m_removeConstraintList(removeConstraintList),
      m_automatic_vertex_constraining(automatic_vertex_constraining)
    {}

    /** const flag for the type of the mass constraint */
    const bool  m_massConstraintType;

    /** list of pdg codes to mass constrain */
    const std::vector<int> m_massConstraintListPDG;

    /** list of pdg codes to mass constrain */
    const std::vector<int>  m_fixedToMotherVertexListPDG;

    /** list of pdg codes to mass constrain */
    const std::vector<int>  m_geoConstraintListPDG;

    /** list of constraints not to apply in tree fit */
    const std::vector<std::string>  m_removeConstraintList;

    /** automatically determing if it is worth extrating a vertex for this particle or if it should be joined with the mother */
    const bool  m_automatic_vertex_constraining;
  };
}

