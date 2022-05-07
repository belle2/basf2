/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/ContinuumSuppression/CleoCones.h>


namespace Belle2 {

  CleoCones::CleoCones(const std::vector<ROOT::Math::XYZVector>& p3_cms_all,
                       const std::vector<ROOT::Math::XYZVector>& p3_cms_roe,
                       const ROOT::Math::XYZVector& thrustB,
                       bool calc_CleoCones_with_all,
                       bool calc_CleoCones_with_roe
                      )
  {
    m_cleo_cone_with_all.clear();
    m_cleo_cone_with_roe.clear();

    // ----------------------------------------------------------------------
    // Calculate momentum flow in 9 cones for all particles in event
    // ----------------------------------------------------------------------
    if (calc_CleoCones_with_all == true) {
      for (int i = 1; i <= 9; i++) {
        float momentum_flow_all = 0;
        for (auto& iter0 : p3_cms_all) {

          /* Use the following intervals
             0*10<= <1*10  0- 10   170-180  180-1*10< <=180-0*10
             1*10<= <2*10 10- 20   160-170  180-2*10< <=180-1*10
             2*10<= <3*10 20- 30   150-160  180-3*10< <=180-2*10
             3*10<= <4*10 30- 40   140-150  180-4*10< <=180-3*10
             4*10<= <5*10 40- 50   130-140  180-5*10< <=180-4*10
             5*10<= <6*10 50- 60   120-130  180-6*10< <=180-5*10
             6*10<= <7*10 60- 70   110-120  180-7*10< <=180-6*10
             7*10<= <8*10 70- 80   100-110  180-8*10< <=180-7*10
             8*10<= <9*10 80- 90    90-100  180-9*10< <=180-8*10
             ==90 */

          float angle = ((180 * acos(thrustB.Unit().Dot(iter0.Unit()))) / M_PI);
          if (((((i - 1) * 10) <= angle) && (angle < (i * 10))) || (((180 - (i * 10)) < angle) && (angle <= (180 - ((i - 1) * 10))))) {
            momentum_flow_all += iter0.R();
            // B2DEBUG(19, "interval " << ((i-1)*10) << " to " << (i*10) << " and " << (180-(i*10)) << " to " << (180-((i-1)*10)) << " has value " << (180*(thrustB.angle(*iter0)))/M_PI << ", momentum flow is " << momentum_flow );
          }
          if ((i == 9) && (angle == 90)) {
            momentum_flow_all += iter0.R();
          }
        }
        m_cleo_cone_with_all.push_back(momentum_flow_all);
      }
    }

    // ----------------------------------------------------------------------
    // Calculate momentum flow in 9 cones for all particles in rest of event
    // ----------------------------------------------------------------------
    if (calc_CleoCones_with_roe == true) {
      for (int i = 1; i <= 9; i++) {
        float momentum_flow_roe = 0;
        for (auto& iter1 : p3_cms_roe) {
          float angle = ((180 * acos(thrustB.Unit().Dot(iter1.Unit()))) / M_PI);
          if (((((i - 1) * 10) <= angle) && (angle < (i * 10))) || (((180 - (i * 10)) < angle) && (angle <= (180 - ((i - 1) * 10))))) {
            momentum_flow_roe += iter1.R();
          }
          if ((i == 9) && (angle == 90)) {
            momentum_flow_roe += iter1.R();
          }
        }
        m_cleo_cone_with_roe.push_back(momentum_flow_roe);
      }
    }
  }

} // Belle2 namespace
