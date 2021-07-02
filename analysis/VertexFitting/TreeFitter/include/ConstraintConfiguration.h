/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
                             m_geoConstraintListPDG({}),
                             m_removeConstraintList({}),
                             m_automatic_vertex_constraining(false),
                             m_ipConstraint(false),
                             m_customOrigin(false),
                             m_customOriginVertex({}),
                             m_customOriginCovariance({}),
                             m_originDimension(3),
                             m_headOfTreePDG(0),
                             m_inflationFactorCovZ(1)
    {};

    /** constructor */
    ConstraintConfiguration(const bool& massConstraintType,
                            const std::vector<int>& massConstraintListPDG,
                            const std::vector<int>& fixedToMotherVertexListPDG,
                            const std::vector<int>& geoConstraintListPDG,
                            const std::vector<std::string>& removeConstraintList,
                            const bool& automatic_vertex_constraining,
                            const bool& ipConstraint,
                            const bool& customOrigin,
                            const std::vector<double>& customOriginVertex,
                            const std::vector<double>& customOriginCovariance,
                            const int& originDimension,
                            const int& inflationFactorCovZ = 1
                           ) :
      m_massConstraintType(massConstraintType),
      m_massConstraintListPDG(massConstraintListPDG),
      m_fixedToMotherVertexListPDG(fixedToMotherVertexListPDG),
      m_geoConstraintListPDG(geoConstraintListPDG),
      m_removeConstraintList(removeConstraintList),
      m_automatic_vertex_constraining(automatic_vertex_constraining),
      m_ipConstraint(ipConstraint),
      m_customOrigin(customOrigin),
      m_customOriginVertex(customOriginVertex),
      m_customOriginCovariance(customOriginCovariance),
      m_originDimension(originDimension),
      m_headOfTreePDG(0),
      m_inflationFactorCovZ(inflationFactorCovZ)
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

    /** automatically determining if it is worth extracting a vertex for this particle or if it should be joined with the mother */
    const bool  m_automatic_vertex_constraining;

    /** is IP constraint  */
    const bool m_ipConstraint;

    /** is custom origin */
    const bool m_customOrigin;

    /** x,y,z of custom origin as vector len 3 */
    const std::vector<double> m_customOriginVertex;

    /** covariance matrix of origin as row major vector len = 9 */
    const std::vector<double> m_customOriginCovariance;

    /** dimension of the origin constraint and ALL geometric gcosntraints */
    const int m_originDimension;

    /** PDG code of the head particle */
    mutable int m_headOfTreePDG;

    /** inflate covariance of z by this number -> iptube  */
    const int m_inflationFactorCovZ;
  };
}

