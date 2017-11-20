/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 **************************************************************************/

#include <analysis/utility/DecayTree.h>

#include <framework/logging/Logger.h>

#include <iostream>
#include <sstream>
#include <cassert>

using namespace Belle2;

DecayTree::DecayTree(const std::string& decaystring, bool removeRadiativeGammaFlag) : m_i(0), m_token_count(0),
  m_match_symbol_position(-1)
{

  m_valid = decaystring.find("No match") == std::string::npos;
  if (m_valid) {
    const auto& root_nodes = this->build_tree(decaystring, removeRadiativeGammaFlag);
    if (root_nodes.size() == 1)
      m_root_node = root_nodes[0];
    else
      m_valid = false;
  }

  // Build cache
  if (m_valid) {
    build_cache(m_root_node);
  }

}

DecayTree::DecayTree(const DecayTree& tree):
  m_root_node(tree.m_root_node)
{
  m_valid = tree.m_valid;
  m_token_count = tree.m_token_count;
  m_match_symbol_position = tree.m_match_symbol_position;
  m_i = tree.m_i;

  if (m_valid) {
    build_cache(m_root_node);
  }
}

DecayTree& DecayTree::operator=(const DecayTree& tree)
{
  m_valid = tree.m_valid;
  m_token_count = tree.m_token_count;
  m_match_symbol_position = tree.m_match_symbol_position;
  m_root_node = tree.m_root_node;
  m_i = tree.m_i;

  if (m_valid) {
    build_cache(m_root_node);
  }

  return *this;
}

void DecayTree::build_cache(DecayNode& node)
{
  m_nodes_cache.push_back(&node);
  for (auto& daughter : node.daughters) {
    build_cache(daughter);
  }
}


std::vector<DecayNode> DecayTree::build_tree(const std::string& decaystring, bool removeRadiativeGammaFlag)
{

  const unsigned int N = decaystring.size();
  std::vector<DecayNode> nodes;
  nodes.reserve(5);

  for (; m_i < N; ++m_i) {
    // Skip ' ', '^' and "-->"
    if (decaystring[m_i] == ' ' or decaystring[m_i] == '>' or (m_i - 1 < N and decaystring[m_i] == '-' and (decaystring[m_i + 1] == '-'
                                                               or decaystring[m_i + 1] == '>'))) {
      continue;
    }

    // Match symbol
    if (decaystring[m_i] == '^') {
      m_match_symbol_position = m_token_count;
      continue;
    }

    // Handle start of subdecay of the form X ( ... )
    if (decaystring[m_i] == '(') {
      m_i++;
      nodes.back().daughters = this->build_tree(decaystring, removeRadiativeGammaFlag);
      continue;
    }

    // Handle end of subdecay
    if (decaystring[m_i] == ')') {
      return nodes;
    }

    // Search end of token
    unsigned int j = m_i + 1;
    for (; j < N; ++j) {
      if (decaystring[j] == ' ' or decaystring[j] == ')' or decaystring[j] == '(')
        break;
    }

    // Parse token and add it as new node
    try {
      const int pdg = std::stoi(decaystring.substr(m_i, j - m_i));
      if (removeRadiativeGammaFlag) {
        if (nodes.size() < 2 or pdg != 22) {
          nodes.emplace_back(pdg);
          m_token_count++;
        }
      } else {
        nodes.emplace_back(pdg);
        m_token_count++;
      }
      m_i = j - 1;
    } catch (const std::invalid_argument&) {
      // We ignore if the token cannot be parsed
    }
  }

  return nodes;
}

bool DecayTree::find_decay(const DecayTree& tree) const
{
  return m_root_node.find_decay(tree.m_root_node);
}

DecayNode& DecayTree::getDecayNode(unsigned int n)
{
  return *(m_nodes_cache[n]);
}

const DecayNode& DecayTree::getDecayNode(unsigned int n) const
{
  return *(m_nodes_cache[n]);
}


std::string DecayTree::to_string() const
{
  return m_root_node.print_node();
}
