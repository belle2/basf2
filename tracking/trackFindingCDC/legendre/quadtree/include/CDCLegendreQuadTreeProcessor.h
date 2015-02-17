/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include "tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h"

#include "tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h"

#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreConformalPosition.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>

#include <framework/logging/Logger.h>

#include <boost/math/constants/constants.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/multi_array.hpp>

#include <set>
#include <vector>
#include <array>
#include <algorithm>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

//     class QuadTreeNeighborFinder;

    template<typename typeX, typename typeY, class typeData>
    class QuadChildrenTemplate;

    template<typename typeX, typename typeY, class typeData>
    class QuadTreeTemplate;

    typedef QuadTreeTemplate<double, int, TrackHit> QuadTreeLegendre;

    class FastHough;
    class TrackHit;


    class QuadTreeProcessor {

    public:

      QuadTreeProcessor() {;};

      static void fillTree(QuadTreeTemplate<double, int, TrackHit>* node, QuadTreeTemplate<double, int, TrackHit>::CandidateProcessorLambda& lmdProcessor) {
        B2DEBUG(100, "startFillingTree with " << node->getItemsVector().size() << "hits at level " << node->getLevel());
        if (node->getItemsVector().size() < node->getNItemsThreshold())
          return;
//        if (node->getRMin() * node->getRMax() >= 0 && fabs(node->getRMin()) > node->getRThreshold()
//            && fabs(node->getRMax()) > node->getRThreshold())
//          return;
        if (node->getLevel() == node->getLastLevel()) {
          lmdProcessor(node);
          return;
        }

        if (node->getChildren() == nullptr)
          node->initializeChildren<QuadTreeProcessor>();

        if (!node->checkFilled()) {
          fillChildren(node->getChildren(), node->getItemsVector());
          node->setFilled();
        }

        int m_nbins_theta = node->getYNbins();
        int m_nbins_r = node->getXNbins();

        bool binUsed[m_nbins_theta][m_nbins_r];
        for (int ii = 0; ii < m_nbins_theta; ii++)
          for (int jj = 0; jj < m_nbins_r; jj++)
            binUsed[ii][jj] = false;

        //Processing, which bins are further investigated
        for (int bin_loop = 0; bin_loop < m_nbins_theta * m_nbins_r; bin_loop++) {
          int t_index = 0;
          int r_index = 0;
          float max_value_temp = 0;
          for (int t_index_temp = 0; t_index_temp < m_nbins_theta; ++t_index_temp) {
            for (int r_index_temp = 0; r_index_temp < m_nbins_r; ++r_index_temp) {
              if ((max_value_temp < node->getChildren()->get(t_index_temp, r_index_temp)->getNItems())
                  && (!binUsed[t_index_temp][r_index_temp])) {
                max_value_temp = node->getChildren()->get(t_index_temp, r_index_temp)->getNItems();
                t_index = t_index_temp;
                r_index = r_index_temp;
              }
            }
          }

          binUsed[t_index][r_index] = true;

          node->getChildren()->get(t_index, r_index)->cleanUpItems<QuadTreeProcessor>();
          node->getChildren()->get(t_index, r_index)->startFillingTree<QuadTreeProcessor>(lmdProcessor);

        }
      }


      static void fillChildren(QuadChildrenTemplate<double, int, TrackHit>* m_children, std::vector<TrackHit*>& m_hits) {

        float r_temp, r_1, r_2;
        int sizeX = m_children->m_sizeX;
        int sizeY = m_children->m_sizeY;
        float dist_1[sizeX + 1][sizeY + 1];
        float dist_2[sizeX + 1][sizeY + 1];
        float m_r;
        unsigned int m_theta;

        //Voting within the four bins
        for (TrackHit * hit : m_hits) {
          //B2DEBUG(100, "PROCCESSING hit " << hit_counter << " of " << nhitsToReserve);
          if (hit->getHitUsage() != TrackHit::not_used)
            continue;
          for (int t_index = 0; t_index < sizeX + 1; ++t_index) {

            // for bin overlapping!
            if (t_index < sizeX)
              m_theta = m_children->get(t_index, 0)->getYMin();
            else
              m_theta = m_children->get(t_index - 1, 0)->getYMax();


            r_temp = hit->getConformalX()
                     * TrigonometricalLookupTable::Instance().cosTheta(m_theta)
                     + hit->getConformalY()
                     * TrigonometricalLookupTable::Instance().sinTheta(m_theta);

            r_1 = r_temp + hit->getConformalDriftLength();
            r_2 = r_temp - hit->getConformalDriftLength();

            //calculate distances of lines to horizontal bin border
            for (int r_index = 0; r_index < sizeY + 1; ++r_index) {
              if (r_index < sizeY)
                m_r = static_cast<float>(m_children->get(0, r_index)->getXMin());
              else
                m_r = static_cast<float>(m_children->get(0, r_index - 1)->getXMax());

              dist_1[t_index][r_index] = m_r - r_1;
              dist_2[t_index][r_index] = m_r - r_2;
            }
          }

          //B2DEBUG(100, "VOTING for hit " << hit_counter << " of " << nhitsToReserve);
          //actual voting, based on the distances (test, if line is passing though the bin)
          for (int t_index = 0; t_index < sizeX; ++t_index) {
            for (int r_index = 0; r_index < sizeY; ++r_index) {
              //curves are assumed to be straight lines, might be a reasonable assumption locally
              if (!FastHough::sameSign(dist_1[t_index][r_index],
                                       dist_1[t_index][r_index + 1], dist_1[t_index + 1][r_index],
                                       dist_1[t_index + 1][r_index + 1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_children->get(t_index, r_index)->insertItem(hit);
              } else if (!FastHough::sameSign(dist_2[t_index][r_index],
                                              dist_2[t_index][r_index + 1], dist_2[t_index + 1][r_index],
                                              dist_2[t_index + 1][r_index + 1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_children->get(t_index, r_index)->insertItem(hit);
              }
            }
          }
          //B2DEBUG(100, "MOVING to next hit");
        }
      }

      static void initializeChildren(QuadTreeTemplate<double, int, TrackHit>* node, QuadChildrenTemplate<double, int, TrackHit>* m_children) {
        for (int i = 0; i < 2; ++i) {
          //m_children[i] = new QuadTreeTemplate*[m_nbins_r];
          for (int j = 0; j < 2; ++j) {
            if (node->getLevel() < (node->getLastLevel() - 5)) {
              m_children->set(i, j, new QuadTreeTemplate<double, int, TrackHit>(node->getXBin(j), node->getXBin(j + 1), node->getYBin(i),
                              node->getYBin(i + 1), node->getLevel() + 1, node));
              m_children->get(i, j)->setLastLevel(node->getLastLevel());
              m_children->get(i, j)->setNItemsThreshold(node->getNItemsThreshold());
            } else {
              double r1 = node->getXBin(j) - fabs(node->getXBin(j + 1) - node->getXBin(j)) / 4.;
              double r2 = node->getXBin(j + 1) + fabs(node->getXBin(j + 1) - node->getXBin(j)) / 4.;
              //        double r1 = m_r[j];
              //        double r2 = m_r[j+1];
              //            theta_1_overlap = thetaBin[t_index]/* - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
              //            theta_2_overlap = thetaBin[t_index + 1]/* + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
              //            theta_1_overlap = thetaBin[t_index] - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
              //            theta_2_overlap = thetaBin[t_index + 1] + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
              int theta1 = node->getYBin(i) - abs(pow(2, node->getLastLevel() + 0 - node->getLevel()) / 4);
              int theta2 = node->getYBin(i + 1)
                           + abs(pow(2, node->getLastLevel() + 0 - node->getLevel()) / 4);
              //        typeY theta1 = m_thetaBin[i];
              //        int theta2 = m_thetaBin[i + 1];

              if (theta1 < 0)
                theta1 = node->getYBin(i);
              if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
                theta2 = node->getYBin(i + 1);

              m_children->set(i, j, new QuadTreeTemplate<double, int, TrackHit>(r1, r2, theta1, theta2, node->getLevel() + 1,
                              node));
              m_children->get(i, j)->setLastLevel(node->getLastLevel());
              m_children->get(i, j)->setNItemsThreshold(node->getNItemsThreshold());
            }
          }
        }
      }

      static void provideItemsSet(const std::set<TrackHit*>& hits_set, std::vector<TrackHit*>& hits_vector) {
        hits_vector.clear();
        std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector),
        [&](TrackHit * hit) {return (hit->getHitUsage() == TrackHit::not_used);});
      };

      static void cleanUpItems(std::vector<TrackHit*>& hits) {
        hits.erase(
          std::remove_if(hits.begin(), hits.end(),
        [&](TrackHit * hit) {return hit->getHitUsage() != TrackHit::not_used;}),
        hits.end());
      } ;


    private:

    };
  }
}
