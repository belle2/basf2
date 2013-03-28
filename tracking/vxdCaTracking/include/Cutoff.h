/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CUTOFF_H
#define CUTOFF_H

#include <algorithm>
#include <string>
#include <vector>

namespace Belle2 {

  class Cutoff;

  class Cutoff {
    /** Description:
     * stores the CutOffValues defined for a special constellation of combined VXDTFHits.
     * The constellation is stored in the scope-variable, the type of CutOff is stored in the type-variable.
     * maybe needed later:
     * Information about the scope of momentum (several ways to implement that, per map or per member variable within this class)
     * */
  public:
    /** constructor **/
    Cutoff(std::string type, std::pair<double, double> values):
      m_type(type),
      m_min(values.first),
      m_max(values.second) {}

    /** getter **/
    const std::string getType() { return m_type; } /**< returns type of cutoff */
    double getMinValue() { return m_min; } /**< returns minValue of cutoff */
    double getMaxValue() { return m_max; } /**< returns maxValue of cutoff */
    const std::vector< std::pair<std::string, double> > getQuantiles() { return m_quantiles; } /**< returns quantiles (extended version of min- and max-value, currently not used ) */

    /** setter **/
    void addValuePair(double minValue, double maxValue) { m_min = minValue; m_max = maxValue; } /**< add a pair of <min, max> */
    void addQuantile(std::pair<std::string, double> newQuantile); /**< add a quantile (currently not used) */

  protected:
    std::string m_type; /**< defines type of cutoff */

    double m_min; /**< min value */
    double m_max; /**< max value */

    std::vector<std::pair<std::string, double> > m_quantiles; /**< extended version of only min and maxValue (quantiles) */
  };


} //Belle2 namespace
#endif
