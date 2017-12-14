// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */
#ifndef EUTELROI_H
#define EUTELROI_H 1

// eutelescope includes ".h"
#include <eutel/eudaq/EUTELESCOPE.h>

namespace eutelescope {


  //! EUTelescope Region of interest
  /*! A region of interest (ROI) is a part of the detector surface the user
   *  wants to keep under control. A typical usage for this is to
   *  select just events occurring inside or outside this region.
   *
   *  The ROI is rectangular and is defined using its bottom left and
   *  top right corners. Those numbers are floats even if in many
   *  cases, ROI's are defined using pixel numbers.
   *
   *  Another additional number (the detector ID) can be set and used
   *  in the case of a multi detector system.
   *
   *  Two utility function to check if a point is inside the ROI are
   *  also provided.
   *
   *  @author Antonio Bulgheroni, INFN  <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTelROI.h,v 1.2 2008-08-19 15:42:47 bulgheroni Exp $
   */
  class EUTelROI {

  public:
    //! Default constructor with the two corners
    /*! A part of assign all the value to the corresponding data
     *  members, it also check the consistency of the provided data
     *
     *  @throw InvalidParameterException if it fails
     */
    EUTelROI(float xBottomLeft, float yBottomLeft, float xTopRight, float yTopRight)    ;

    //! Default constructor with the two corners and the detector ID
    /*! A part of assign all the value to the corresponding data
     *  members, it also check the consistency of the provided data
     *
     *  @throw InvalidParameterException if it fails
     */
    EUTelROI(int detectorID, float xBottomLeft, float yBottomLeft, float xTopRight, float yTopRight);

    //! Get the two corners
    /*! This method allow to access to the boundaries of the ROI.
     *
     *  @param xBottomLeft A pointer to the bottom left x
     *  @param yBottomLeft A pointer to the bottom left y
     *  @param xTopRight   A pointer to the top right x
     *  @param yTopRight   A pointer to the top right y
     */
    void getCorners(float* xBottomLeft, float* yBottomLeft, float* xTopRight, float* yTopRight) const ;

    //! Get the detector identification number
    /*! In the case the ROI belongs to a specific detector, this
     *  method returns the detector ID. In the case the detector ID
     *  was not set, then numeric_limits<int>::min is returned
     *
     *  @return The detector identification number or
     *  numeric_limits<int>::min if not set.
     */
    int  getDetectorID() const;

    //! Check if the point is inside
    /*! This utility method checks if the point defined by @c x and @c
     *  y is inside a the ROI or not
     *
     *  @param x Coordinate x of the point under test
     *  @param y Coordinate y of the point under test
     *  @return True if the point under test is inside the ROI
     */
    bool isInside(float x, float y) const ;

    //! Check if the point is inside and on the same sensor
    /*! This utility method checks if the point under test is inside
     *  the ROI and on the same detector.
     *
     *  @param x Coordinate x of the point under test
     *  @param y Coordinate y of the point under test
     *  @param detectorID The detector the point under test belongs
     *  to.
     *  @return True if the point under test is inside the ROI and on
     *  the same detector
     */
    bool isInside(int detectorID, float x, float y) const ;

    //! Streamer
    /*! Streamer method to output the content of a EUTelROI.
     *
     *  @param os The input ostream
     *  @param roi The EUTelROI to be streamed
     *  @return The output stream
     */
    friend std::ostream& operator<< (std::ostream& os, EUTelROI roi) ;

  private:

    //! Consistency check
    /*! This private method is used to check the consistency of the
     *  data provided by the user. If the bottom corner is above the
     *  top one or similar an InvalidParameter exception is thrown.
     *
     *  throw InvalidParameterException is the consistency check fails
     */
    void consistencyCheck() const;

    //! Bottom left x
    /*! This is the bottom left x coordinate of the ROI
     */
    float _xBottomLeft;

    //! Bottom left y
    /*! This is the bottom left y coordinate of the ROI
     */
    float _yBottomLeft;

    //! Top right x
    /*! This is the top right x coordinate of the ROI
     */
    float _xTopRight;

    //! Top right y
    /*! This is the top right y coordinate of the ROI
     */
    float _yTopRight;

    //! Detector ID
    /*! This is the detector ID in the geometry description. It is an
     *  optional parameter.
     */
    int _detectorID;


  };

}
#endif
