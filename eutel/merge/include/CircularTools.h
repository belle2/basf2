/*
 * CircularTools.h
 *
 *  Created on: Jan 30, 2014
 *      Author: kvasnicka
 */

#ifndef CIRCULARTOOLS_H_
#define CIRCULARTOOLS_H_

#include <algorithm>
#include <functional>
#include <set>

namespace CIRC {

  /*
   * Tools to work with circular-type EUDET TLU tags.
   * The tools are not general and may not ensure proper ordering in general case,
   * but are expected to work with sets of small range compared to the tag range.
   */

  typedef int tag_type;

  enum {
    c_tagLowerBound = 0,
    c_tagUpperBound = 32768
  };

  /** Wrap value to interval <c_tagLowerBound, c_tagUpperBound)
   * @param tag Value to be wrapped
   * @return Wrapped value
   */
  inline tag_type wrap(tag_type tag)
  {
    while (tag < c_tagLowerBound) tag += c_tagUpperBound;
    while (tag >= c_tagUpperBound) tag -= c_tagUpperBound;
    return tag;
  }

  /** Typedef for the compare function type */
  typedef std::function<bool (tag_type, tag_type)> compare_type;

  /** Compare function for tag_type.
   * tag1 is less than tag2, if the round-a-clock distance from tag1 to tag2 is less
   * than the distance from tag2 to tag1. For convenience, if the distances are equal,
   * the nominally smaller value is the smaller one.
   */
  inline bool compare(tag_type tag1, tag_type tag2)
  {
    tag_type dist1 = wrap(tag1 - tag2);
    tag_type dist2 = wrap(tag2 - tag1);
    if (dist1 > dist2) return true;
    else return false;
  }

  /** Calculate distance between two tag values.
   * @param tag1 First value
   * @param tag2 second value
   * @return Distance between the values.
   */
  inline std::size_t distance(tag_type tag1, tag_type tag2)
  {
    tag_type t1 = wrap(tag1);
    tag_type t2 = wrap(tag2);
    if (t1 > t2) std::swap(t1, t2);
    return std::size_t(std::min(t2 - t1, t1 + c_tagUpperBound - t2));
  }

} // namespace CIRC

#endif /* CIRCULARTOOLS_H_ */
