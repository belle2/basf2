// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */

#ifndef EUTELBASEDETECTOR_H
#define EUTELBASEDETECTOR_H

// personal includes ".h"
#include <eutel/eudaq/EUTELESCOPE.h>

// lcio includes <.h>

// system includes <>
#include <iostream>
#include <string>

namespace eutelescope {


  //! Virtual class to describe detector in the EUTelescope framework
  /*!
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTelBaseDetector.h,v 1.4 2008-08-20 12:50:40 bulgheroni Exp $
   */

  class EUTelBaseDetector {

  public:
    //! Default constructor
    EUTelBaseDetector() { }

    //! Default destructor
    virtual ~EUTelBaseDetector() {;}

    //! Print
    /*! This method is used to print out the detector
     *
     *  @param os The input output stream
     */
    virtual void print(std::ostream& os) const                            = 0;

    //! Overload of operator<<
    /*! This friend function is the overload of the operator << for
     *  the detector base class. It uses the print method that is
     *  virtually defined for all cluster subclasses.
     *
     *  @param os The input output stream as modified by the print
     *  method
     *  @param clu The detector to be stream out
     *  @return The output stream
     */
    friend std::ostream& operator<< (std::ostream& os , const EUTelBaseDetector& clu)  { clu.print(os); return os; }

  protected:

    // data members

    //! This is the detector name!
    std::string _name;


  };

}

#endif

//  LocalWords:  iostream destructor
