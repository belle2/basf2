/**
 * Thomas Keck 2014
 */

#pragma once
#include <tracking/trackFindingVXD/tmpFastBDT/FastBDT.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <type_traits>

namespace tmpFastBDT {

  /**
   * This template saves a vector to an std::ostream
   * @param stream an std::ostream reference
   * @param vector the vector which shall be stored
   */
  template<class T>
  std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vector)
  {
    stream << vector.size();
    for (const auto& value : vector) {
      stream << " " << value;
    }
    stream << std::endl;
    return stream;
  }

  /**
   * Specialize vector output operator, so it checks for nan and infinity in float/double types
   * Note: I know about http://www.gotw.ca/publications/mill17.htm, SFINAE, but nothing worked for me ...
   *       so I sticked with this simple solution instead of complicated template meta programming
   */
  template<>
  std::ostream& operator<<(std::ostream& stream, const std::vector<float>& vector);

  template<>
  std::ostream& operator<<(std::ostream& stream, const std::vector<double>& vector);

  /**
   * This template reads a vector from an std::istream
   * @param stream an std::istream reference
   * @param vector the vector containing read data
   */
  template<class T>
  std::istream& operator>>(std::istream& stream, std::vector<T>& vector)
  {
    unsigned int size;
    stream >> size;
    vector.resize(size);
    for (unsigned int i = 0; i < size; ++i)
      stream >> vector[i];
    return stream;
  }

  /**
   * This function saves a Cut to an std::ostream
   * @param stream an std::ostream reference
   * @param cut which shall be stored
   */
  std::ostream& operator<<(std::ostream& stream, const Cut& cut);

  /**
   * This function reads a Cut from an std::istream
   * @param stream an std::istream reference
   * @param cut containing read data
   */
  std::istream& operator>>(std::istream& stream, Cut& cut);

  /**
   * This function saves a Tree to an std::ostream
   * @param stream an std::ostream reference
   * @param tree the tree which shall be stored
   */
  std::ostream& operator<<(std::ostream& stream, const Tree& tree);

  /**
   * This function reads a Tree from an std::istream
   * @param stream an std::istream reference
   * @preturn tree containing read data
   */
  Tree readTreeFromStream(std::istream& stream);

  /**
   * This function saves a Forest to an std::ostream
   * @param stream an std::ostream reference
   * @param forest the forest which shall be stored
   */
  std::ostream& operator<<(std::ostream& stream, const Forest& forest);

  /**
   * This function reads a Forest from an std::istream
   * @param stream an std::istream reference
   * @preturn forest containing read data
   */
  Forest readForestFromStream(std::istream& stream);

  /**
   * This function saves a FeatureBinning to an std::ostream
   * @param stream an std::ostream reference
   * @param featureBinning the FeatureBinning which shall be stored
   */
  template<class T>
  std::ostream& operator<<(std::ostream& stream, const FeatureBinning<T>& featureBinning)
  {

    stream << featureBinning.GetNLevels() << std::endl;
    stream << featureBinning.GetBinning() << std::endl;

    return stream;
  }

  /**
   * This function reads a FeatureBinning from an std::istream
   * @param stream an std::istream reference
   * @preturn FeatureBinning containing read data
   */
  template<class T>
  FeatureBinning<T> readFeatureBinningFromStream(std::istream& stream)
  {

    unsigned int nLevels;
    stream >> nLevels;

    std::vector<T> bins;
    stream >> bins;

    return FeatureBinning<T>(nLevels, bins.begin(), bins.end());

  }

  /**
   * Overload vector input operator, so it can read in FeatureBinnings
   */
  template<class T>
  std::istream& operator>>(std::istream& stream, std::vector<FeatureBinning<T>>& vector)
  {
    unsigned int size;
    stream >> size;
    for (unsigned int i = 0; i < size; ++i)
      vector.push_back(readFeatureBinningFromStream<T>(stream));
    return stream;
  }


}
