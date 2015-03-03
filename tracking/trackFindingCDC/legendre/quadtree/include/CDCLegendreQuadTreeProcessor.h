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

    typedef QuadTreeTemplate<int, double, TrackHit> QuadTreeLegendre;

    class FastHough;
    class TrackHit;


    class QuadTreeProcessor {

    public:

      QuadTreeProcessor() {;};

      static void fillTree(QuadTreeTemplate<int, double, TrackHit>* node, QuadTreeTemplate<int, double, TrackHit>::CandidateProcessorLambda& lmdProcessor) {
        B2DEBUG(100, "startFillingTree with " << node->getItemsVector().size() << "hits at level " << node->getLevel());
        if (node->getItemsVector().size() < node->getNItemsThreshold())
          return;
//        if ((node->getXMin() * node->getXMax() >= 0) && (fabs(node->getXMin()) > rThreshold)  && (fabs(node->getXMax()) > rThreshold))
//          return;
        if (node->getLevel() == node->getLastLevel()) {
          lmdProcessor(node);
          return;
        }

        if (node->getChildren() == nullptr)
          node->initializeChildren<QuadTreeProcessor>();

        if (!node->checkFilled()) {
          fillChildren(node, node->getItemsVector());
          node->setFilled();
        }

        int m_nbins_theta = node->getXNbins();
        int m_nbins_r = node->getYNbins();

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

          node->getChildren()->get(t_index, r_index)->setNItemsThreshold(node->getNItemsThreshold());
          node->getChildren()->get(t_index, r_index)->cleanUpItems<QuadTreeProcessor>();
          node->getChildren()->get(t_index, r_index)->startFillingTree<QuadTreeProcessor>(lmdProcessor);

        }
      }

      static void fillGivenTree(QuadTreeTemplate<int, double, TrackHit>* node, QuadTreeTemplate<int, double, TrackHit>::CandidateProcessorLambda& lmdProcessor, unsigned int m_nHitsThreshold, double m_rThreshold) {
        B2DEBUG(100, "startFillingTree with " << node->getItemsVector().size() << "hits at level " << node->getLevel());
        if (node->getItemsVector().size() < m_nHitsThreshold)
          return;
        if ((node->getYMin() * node->getYMax() >= 0) && (fabs(node->getYMin()) > m_rThreshold)  && (fabs(node->getYMax()) > m_rThreshold))
          return;
        if (node->getLevel() == node->getLastLevel()) {
          lmdProcessor(node);
          return;
        }

        if (node->getChildren() == nullptr)
          node->initializeChildren<QuadTreeProcessor>();

        if (!node->checkFilled()) {
          fillChildren(node, node->getItemsVector());
          node->setFilled();
        }

        int m_nbins_theta = node->getXNbins();
        int m_nbins_r = node->getYNbins();

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

//          node->getChildren()->get(t_index, r_index)->setNItemsThreshold(node->getNItemsThreshold());
          node->getChildren()->get(t_index, r_index)->cleanUpItems<QuadTreeProcessor>();
//          node->getChildren()->get(t_index, r_index)->startFillingTree<QuadTreeProcessor>(lmdProcessor);
          QuadTreeProcessor::fillGivenTree(node->getChildren()->get(t_index, r_index), lmdProcessor, m_nHitsThreshold, m_rThreshold);
        }
      }

      static void fillChildren(QuadTreeTemplate<int, double, TrackHit>* m_node, std::vector<TrackHit*>& m_hits) {

        int sizeX = 2;
        int sizeY = 2;
        double dist_1[3][3];
        double dist_2[3][3];
        const size_t neededSize = 2 * m_hits.size();
        m_node->getChildren()->apply([neededSize](QuadTreeTemplate<int, double, TrackHit>* qt) {qt->reserveHitsVector(neededSize);});



        //Voting within the four bins
        for (TrackHit * hit : m_hits) {
          //B2DEBUG(100, "PROCCESSING hit " << hit_counter << " of " << nhitsToReserve);
          if (hit->getHitUsage() != TrackHit::not_used)
            continue;

          for (int t_index = 0; t_index < sizeX; ++t_index) {
            for (int r_index = 0; r_index < sizeY; ++r_index) {

              double r_temp_min = hit->getConformalX()
                                  * TrigonometricalLookupTable::Instance().cosTheta(m_node->getChildren()->get(t_index, r_index)->getXMin())
                                  + hit->getConformalY()
                                  * TrigonometricalLookupTable::Instance().sinTheta(m_node->getChildren()->get(t_index, r_index)->getXMin());
              double r_temp_max = hit->getConformalX()
                                  * TrigonometricalLookupTable::Instance().cosTheta(m_node->getChildren()->get(t_index, r_index)->getXMax())
                                  + hit->getConformalY()
                                  * TrigonometricalLookupTable::Instance().sinTheta(m_node->getChildren()->get(t_index, r_index)->getXMax());

              double r_min1 = r_temp_min - hit->getConformalDriftLength();
              double r_min2 = r_temp_min + hit->getConformalDriftLength();
              double r_max1 = r_temp_max - hit->getConformalDriftLength();
              double r_max2 = r_temp_max + hit->getConformalDriftLength();

              double m_rMin = static_cast<double>(m_node->getChildren()->get(t_index, r_index)->getYMin());
              double m_rMax = static_cast<double>(m_node->getChildren()->get(t_index, r_index)->getYMax());

              dist_1[0][0] = m_rMin - r_min1;
              dist_1[0][1] = m_rMin - r_max1;
              dist_1[1][0] = m_rMax - r_min1;
              dist_1[1][1] = m_rMax - r_max1;

              dist_2[0][0] = m_rMin - r_min2;
              dist_2[0][1] = m_rMin - r_max2;
              dist_2[1][0] = m_rMax - r_min2;
              dist_2[1][1] = m_rMax - r_max2;


              //curves are assumed to be straight lines, might be a reasonable assumption locally
              if (!FastHough::sameSign(dist_1[0][0], dist_1[0][1],
                                       dist_1[1][0], dist_1[1][1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_node->getChildren()->get(t_index, r_index)->insertItem(hit);
              } else if (!FastHough::sameSign(dist_2[0][0], dist_2[0][1],
                                              dist_2[1][0], dist_2[1][1])) {
                //B2DEBUG(100, "INSERT hit in " << t_index << ";" << r_index << " bin");
                m_node->getChildren()->get(t_index, r_index)->insertItem(hit);
              }



            }
          }
          //B2DEBUG(100, "MOVING to next hit");
        }
      }

      static void initializeChildren(QuadTreeTemplate<int, double, TrackHit>* node, QuadChildrenTemplate<int, double, TrackHit>* m_children) {

        for (int i = 0; i < 2; ++i) {
          //m_children[i] = new QuadTreeTemplate*[m_nbins_r];
          for (int j = 0; j < 2; ++j) {
            if ((node->getLevel() > (node->getLastLevel() - 6)) && (fabs(node->getYMean()) > 0.006)) {
              double r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
              double r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
              //        double r1 = m_r[j];
              //        double r2 = m_r[j+1];
              //            theta_1_overlap = thetaBin[t_index]/* - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
              //            theta_2_overlap = thetaBin[t_index + 1]/* + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.*/;
              //            theta_1_overlap = thetaBin[t_index] - fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
              //            theta_2_overlap = thetaBin[t_index + 1] + fabs(thetaBin[t_index + 1] - thetaBin[t_index]) / 2.;
              int theta1 = node->getXBin(i) - abs(pow(2, node->getLastLevel() + 0 - node->getLevel()) / 4);
              int theta2 = node->getXBin(i + 1)
                           + abs(pow(2, node->getLastLevel() + 0 - node->getLevel()) / 4);
              //        typeY theta1 = m_thetaBin[i];
              //        int theta2 = m_thetaBin[i + 1];

              if (theta1 < 0)
                theta1 = node->getXBin(i);
              if (theta2 >= TrigonometricalLookupTable::Instance().getNBinsTheta())
                theta2 = node->getXBin(i + 1);

              m_children->set(i, j, new QuadTreeTemplate<int, double, TrackHit>(theta1, theta2, r1, r2, node->getLevel() + 1,
                              node));
              m_children->get(i, j)->setLastLevel(node->getLastLevel());
              m_children->get(i, j)->setNItemsThreshold(node->getNItemsThreshold());
            } else {
              m_children->set(i, j, new QuadTreeTemplate<int, double, TrackHit>(node->getXBin(i), node->getXBin(i + 1), node->getYBin(j),
                              node->getYBin(j + 1), node->getLevel() + 1, node));
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


      static double rThreshold;
      static unsigned int nHitsThreshold;


    private:

    };
  }
}
