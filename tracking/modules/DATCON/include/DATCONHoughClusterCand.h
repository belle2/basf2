/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// #include <tracking/modules/DATCON/DATCONModule.h>
// #include <tracking/modules/DATCON/DATCONHoughClusterCand.h>

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <pxd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <boost/array.hpp>
#include <string>
#include <set>
#include <vector>
#include <fstream>

#include <root/TVector3.h>
#include <root/TVector2.h>
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TH1D.h>
#include <root/TH2D.h>
#include <root/TF1.h>

namespace Belle2 {

  /*
  * Hough Cluster Candidate
  */
  class DATCONHoughClusterCand {
  public:
    /** Constructor for hough candidates */
    DATCONHoughClusterCand(unsigned int _size = 0, TVector2 _center = TVector2(0.0, 0.0), TVector2 _pitch = TVector2(0.0,
                           0.0)): size(_size), center(_center), pitch(_pitch)
    {
      size = _size;
      center = _center;
      pitch = _pitch;
      max_x = 4.0;
      max_y = 4.0;
    }

    /** Descructor */
    virtual ~DATCONHoughClusterCand() {}

    /** Get Size */
    unsigned int getSize() { return size; }

    /** Increase Size */
    void incSize() { ++size; }

    /** Set Size */
    void setSize(unsigned int _size) { size = _size; }

    /** Get Center */
    TVector2 getCenter() { return center; }

    /** Add Center */
    void addCenter(TVector2 _addcenter)
    {
      center += _addcenter;

      /* Handle left lower edge */
      if (left.X() > _addcenter.X()) {
        left.Set(_addcenter.X(), left.Y());
      }
      if (left.Y() > _addcenter.Y()) {
        left.Set(left.X(), _addcenter.Y());
      }

      /* Handle right higher edge */
      if (right.X() < _addcenter.X()) {
        right.Set(_addcenter.X(), right.Y());
      }
      if (right.Y() < _addcenter.Y()) {
        right.Set(right.X(), _addcenter.Y());
      }
    }

    /** Set max cluster size */
    void setMaxSize(double x, double y)
    {
      max_x = x;
      max_y = y;
    }

    /** Set Center */
    void setCenter(TVector2 _center)
    {
      center = _center;
      left = center - pitch;
      right = center + pitch;
    }

    TVector2 getGravity()
    {
      TVector2 grav;
      grav.Set(center.X() / size, center.Y() / size);
      return grav;
    }

    /** Get Pitch */
    TVector2 getPitch() { return pitch; }

    /** Set Pitch */
    void setPitch(TVector2 _pitch)
    {
      pitch = _pitch;
    }

    /** Print edges */
    void printEdges()
    {
      B2DEBUG(250, "    Left edge: (" << left.X() << ", " << left.Y() << ") Right edge: (" << right.X() << ", "
              << right.Y() << ")");
    }

    /** Get left and right edges */
    TVector2 getLeftEdge() { return left; }
    TVector2 getRightEdge() { return right; }

    double getSizeX()
    {
      return (right.X() - left.X());
    }

    double getSizeY()
    {
      return (right.Y() - left.Y());
    }

    /** Check Cluster Size */
    bool checkClusterSize(TVector2 _addcenter)
    {
      TVector2 _left, _right;
      /* Handle left lower edge */
      if (left.X() > _addcenter.X()) {
        _left.Set(_addcenter.X(), left.Y());
      }
      if (left.Y() > _addcenter.Y()) {
        _left.Set(left.X(), _addcenter.Y());
      }

      /* Handle right higher edge */
      if (right.X() < _addcenter.X()) {
        _right.Set(_addcenter.X(), right.Y());
      }
      if (right.Y() < _addcenter.Y()) {
        _right.Set(right.X(), _addcenter.Y());
      }

      if ((_right.Y() - left.Y()) < max_y && (_right.X() - left.X()) < max_x) {
        return true;
      }

      return false;
    }

  private:
    /** Size of the cluster */
    unsigned int size;

    /** Size of the cluster */
    TVector2 center;

    /** Pitch */
    TVector2 pitch;

    /** Out edges **/
    TVector2 left;
    TVector2 right;

    /** Max cluster size */
    double max_x, max_y;
  }; // end class definition

}; // end namespace Belle2


