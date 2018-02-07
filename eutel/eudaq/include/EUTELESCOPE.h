#// -*- mode: c++; mode: auto-fill; mode: flyspell-prog -*-
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */

#ifndef EUTELESCOPE_NAMESPACE_H
#define EUTELESCOPE_NAMESPACE_H

//! The eutelescope namespace.
/*! This namespace is used in order not to pollute neither the lcio,
 *  nor the Marlin, not the standard namespaces. It contains all
 *  classes defined by the EUDET JRA1 collaboration in order to
 *  develop both their DAQ and analysis/reconstruction software.
 *
 *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
 *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
 */

namespace eutelescope {}
#endif

#ifndef EUTELESCOPE_H
#define EUTELESCOPE_H

#ifdef USE_EUDAQ
# include <eutel/eudaq/Utils.h>
#else
# include <iomanip>
# include <sstream>
# include <exception>
# include <stdexcept>
#endif

#ifdef USE_MARLIN
// streamlog include
#include "streamlog/streamlog.h"
#endif

// system includes <>
#include <iostream>
#include <string>
#include <vector>

namespace eutelescope {

  //! Global constants used in the Eutelescope package
  /*!
   * This class has only static data members used only to define global
   * constant to be used within the Eutelescope package. Please add here
   * whatever constant you want to use.  A typical useful of this class
   * is to define name of collection to be retrieved/saved from/to
   * files.
   *
   * @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   * @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */

  class EUTELESCOPE {

  public:
    //! Default destructor.
    /*! This is the default destructor, but it is actually a NO-OP
     *  since there is nothing to be destroyed.
     */
    virtual ~ EUTELESCOPE()  {;  }

  public:

    // PARAMETER NAMES USED IN THE HEADER IMPLEMENTATION

    //! Parameter key to store/recall the header version number
    static const char* HEADERVERSION;

    //! Parameter key to store/recall the data type
    /*! The value of DATATYPE can be set using one of the static const
     *  defined in this class. Use DAQDATA for data coming from a real
     *  acquisition; SIMULDATA for data produced by a simulation job;
     *  CONVDATA for data converted from another data format.
     *
     *  @see EUTELESCOPE::DAQDATA, EUTELESCOPE::SIMULDATA, EUTELESCOPE::CONVDATA
     */
    static const char* DATATYPE;

    //! Parameter key to store/recall the number of events in the file
    static const char* NOOFEVENT;

    //! Parameter key to store/recall the date time string in human readable format
    static const char* DATETIME;

    //! Parameter key to store/recall the DAQ hardware name
    /*! The value of the DAQHWNNAME stored in the header of LCIO file
     *  can be one of the DAQ name provided in this class.  @see
     *  EUTELESCOPE::EUDRB, EUTELESCOPE::IPHCIMAGER,
     *  EUTELESCOPE::SUCIMAIMAGER
     */
    static const char* DAQHWNAME;

    //! Parameter key to store/recall the DAQ hardware version
    static const char* DAQHWVERSION;

    //! Parameter key to store/recall the DAQ software name
    /*! This parameter can be set using one of the const string
     *  defined as well in this class (e.g. EUDAQ).
     *
     *  @see EUTELESCOPE::EUDAQ
     */
    static const char* DAQSWNAME;

    //! Parameter key to store/recall the DAQ software version
    static const char* DAQSWVERSION;

    //! Parameter key to store/recall the simulation software name
    static const char* SIMULSWNAME;

    //! Parameter key to store/recall the simulation software version
    static const char* SIMULSWVERSION;

    //! Parameter key to store/recall the geometry identification number
    static const char* GEOID;

    //! Parameter key to store/recall the beam location
    static const char* BEAMLOCATION;

    //! Parameter key to store/recall the beam type
    static const char* BEAMTYPE;

    //! Parameter key to store/recall the beam energy
    static const char* BEAMENERGY;

    //! Parameter key to store/recall the number of detector in the file
    static const char* NOOFDETECTOR;

    //! Parameter key to store/recall the minimum X pixel
    static const char* MINX;

    //! Parameter key to store/recall the maximum X pixel
    static const char* MAXX;

    //! Parameter key to store/recall the minimum Y pixel
    static const char* MINY;

    //! Parameter key to store/recall the maximum Y pixel
    static const char* MAXY;

    //! Parameter key to store/recall the list of applied processors
    static const char* APPLIEDPROCESSOR;

    //! Parameter key to store/recall the intermediate file names
    static const char* INTERMEDIATEFILE;

    //! Parameter key to store/recall the user defined run header comment
    static const char* USERCOMMENT;

    // standard name to be saved into the header

    //! Constant used to identify really acquired data
    static const char* DAQDATA;

    //! Constant used to identify simulated data
    static const char* SIMULDATA;

    //! Constant used to identify data converted from another data format
    static const char* CONVDATA;

    //! Constant used to identify the DAQ board designed by INFN
    static const char* EUDRB;

    //! Constant used to identify the DAQ board developed in IPHC/LEPSI
    static const char* IPHCIMAGER;

    //! Constant used to identify the DAQ board developed by the SUCIMA collaboration
    static const char* SUCIMAIMAGER;

    //! Constant used to identify the DAQ software developed by Geneva team
    static const char* EUDAQ;

    //! Constant used to identify the EUDRB operation mode
    static const char* EUDRBMODE;

    //! Constant used to identify the detectors making the telescope
    static const char* EUDRBDET;

    // PARAMETER NAMES USED IN THE EVENT IMPLEMENTATION

    //! Constant used to get/set the event type
    static const char* EVENTTYPE;


    // pixel flags

    //! Constant to identify good pixels
    static const int GOODPIXEL;

    //! Constant to identify bad pixels
    static const int BADPIXEL;

    //! Constant to identify hit pixels
    static const int HITPIXEL;

    //! Constant to identify missing pixels
    static const int MISSINGPIXEL;

    //! Constant to identify pixels firing too often
    static const int FIRINGPIXEL;

    // algorithm names

    //! Bad pixel masking algorithm identifier
    /*! @see EUTelPedestalNoiseProcessor::maskBadPixel() for a
     *  detailed description of the algorithm
     */
    static const char* NOISEDISTRIBUTION;

    //! Bad pixel masking algorithm identifier
    /*! @see EUTelPedestalNoiseProcessor::maskBadPixel() for a
     * detailed description of the algorithm.
     */
    static const char* ABSOLUTENOISEVALUE;

    //! Bad pixel masking algorithm identifier
    /*! @see EUTelPedestalNoiseProcessor::maskBadPixel() for a
     * detailed description of the algorithm.
     */
    static const char* DEADPIXEL;

    //! Bad pixel masking algorithm identifier
    /*! @see EUTelPedestalNoiseProcessor::maskBadPixel() for a
     * detailed description of the algorithm.
     */
    static const char* ABSOLUTEPEDEVALUE;

    //! Common mode algorithm identifier
    static const char* FULLFRAME;

    //! Common mode algorithm identifier
    static const char* ROWWISE;

    //! Pedestal calculation algorithm identifier
    /*! This string is used to identify a pedestal calculation
     *  algorithm. @a MEANRMS means that the pedestal value is defined
     *  as the mean value of each single pixel signal distribution,
     *  while the noise is given by the distribution RMS. The
     *  algorithm is based on the use of std::vector's
     */
    static const char* MEANRMS;

    //! Pedestal calculation algorithm identifier
    /*! This string is used to identify a pedestal calculation
     *  algorithm. @a AIDAPROFILE means that pedestal and noise are
     *  determined taking advantages from the use of an
     *  AIDA::IProfile2D object. For each event, all pixel signals are
     *  filled into a 2d profile properly booked in the init()
     *  phase. When the loop is complete, the mean value and the noise
     *  of each pixel is dumped from the IProfile2D to pedestal and
     *  noise std::vector's. Of course, the use of it is limited to
     *  the cases in which USE_AIDA is defined. Otherwise, the
     *  algorithm will fall back to something, may be to so elegant,
     *  but definitely more standard (EUTELESCOPE::MEANRMS).
     */
    static const char* AIDAPROFILE;

    //! Fixed frame clustering algorithm
    /*! For a detailed description @see
     *  EUTelClusteringProcessor::_clusteringAlgo
     */
    static const char* FIXEDFRAME;


    //! Digital fixed frame clustering algorithm
    /*! For a detailed description @see
     *  EUTelClusteringProcessor::_clusteringAlgo
     */
    static const char* DFIXEDFRAME;

    //! Clustering algorithm for ZS data
    /*! This is the basic algorithm for clustering together zero
     *  suppress data. It is based on three numbers, the minimum
     *  distance between pixels, the seed SNR and the cluster SNR.
     */
    static const char* SPARSECLUSTER;

    //! clustering algorithm for ZS data with better performance
    /*! This is an advanced clustering algorithm for sparsified pixel.
     *  It is based on two numbers: the seed SNR and the cluster SNR.
     */
    static const char* SPARSECLUSTER2;

    //! Bricked cluster fixed frame algorithm
    /*! For a detailed description @see
     *  EUTelClusteringProcessor::_clusteringAlgo
     *  HACK TAKI
     */
    static const char* BRICKEDCLUSTER;

    //! Analog type of the data readout
    /*! For a detailed description @see
     *  EUTelClusteringProcessor::_dataFormatType
     */
    static const char*    ANALOG;

    //! Digital type of the data readout
    /*! For a detailed description @see
     *  EUTelClusteringProcessor::_dataFormatType
     */
    static const char*    DIGITAL;

    //! Binary type of the data readout
    /*! For a detailed description @see
     *  EUTelClusteringProcessor::_dataFormatType
     */
    static const char*    BINARY;

    //! Fixed weight algorithm for the pedestal / noise update
    /*! The name for the pedestal and noise update algorithm. @see
     *  EUTelUpdatePedestalNoiseProcessor
     */
    static const char* FIXEDWEIGHT;

    //! Cluster separation algorithm with only flagging capability
    /*! The name for the cluster separation algorithm that is not
     *  really dividing merging clusters, but only flagging their
     *  quality. @see EUTelClusterSeparationProcessor
     */
    static const char* FLAGONLY;

    // Encoding strings

    //! Default Tracker(Raw)Data encoding for full matrix
    /*! This constant string is used with CellIDEncoder to define the
     *  default encoding used for describe cells into a
     *  Tracker(Raw)Data object
     *
     *  "sensorID:5,xMin:12,xMax:12,yMin:12,yMax:12"
     */
    static const char* MATRIXDEFAULTENCODING;

    //! Default Tracker(Raw)Data encoding for full matrix extendet by
    //the start gate information
    /*! This constant string is used with CellIDEncoder to define the
     *  default encoding used for describe cells into a
     *  Tracker(Raw)Data object
     *
     *  "sensorID:5,xMin:10,xMax:10,yMin:10,yMax:10,StartGate:8"
     */
    static const char* MATRIXSTARTGATEENCODING;

    //! Default encoding for zero suppress data
    /*! This constant string is used with CellIDEncoder to define the
     *  encoding used for describe cells in a TrackerData object
     *  containing sparsified pixel.
     *
     *  The sparse pixel type is defined using the SparsePixelType
     *  enumeration.
     *
     *  @see SparsePixelType
     *
     *  "sensorID:5,sparsePixelType:5"
     */
    static const char* ZSDATADEFAULTENCODING;

    //! Default TrackerData encoding for cluster
    /*! This constant string is used with CellIDEncoder to define the
     *  default encoding used for describe cells into a clusters. This
     *  encoding is different from the one for complete matrices.
     *
     *  "sensorID:5,clusterID:8,xSeed:12,ySeed:12,xCluSize:5,yCluSize:5:quality:5"
     *
     *  Note about cluster quality: this is a three bit flag to be
     *  used with the cluster quality enum.
     *
     *  @see ClusterQuality
     */
    static const char* CLUSTERDEFAULTENCODING;

    //! Default TrackerPulse encoding for cluster
    /*! This constant string is used with CellIDEncoder to define the
     *  default encoding used to describe cells into a tracker
     *  pulse. This encoding is very similar to CLUSTERDEFAULTENCODING
     *  but instead of the quality it has a 5 bit fields to identify
     *  the cluster reimplementation class.
     *
     *  "sensorID:5,clusterID:8,xSeed:12,ySeed:12,xCluSize:5,yCluSize:5:type:5"
     *
     *  @see ClusterType
     */
    static const char* PULSEDEFAULTENCODING;

    //! Zero suppress cluster default encoding
    /*! This encoding string is used for the TrackerData containing
     *  clusters made by sparsified pixels
     *
     *  "sensorID:5,clusterID:8,sparsePixelType:5,quality:5"
     *
     *  @see SparsePixelType
     *  @see ClusterQuality
     */
    static const char* ZSCLUSTERDEFAULTENCODING;

    //! Zero suppress cluster encoding for ATLAS-Pixel clusters
    /*! This encoding string is used for the TrackerData containing
     *  clusters made by ATLAS PIXEL pixels
     *
     *  "sensorID:17,clusterID:8,sparsePixelType:5"
     *
     *  @see SparsePixelType
     *  @see ClusterQuality
     */
    static const char* ZSAPIXCLUSTERENCODING;


  };


  //! Detector type enum
  /*! This enumeration type is used to identify a specific detector
   *  type.
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */
  enum EUTelDetectorType {
    kTLU               =   0,
    kMimoTel           = 100,
    kMimosa18          = 101,
    kMimosa26          = 102,
    kDEPFET            = 103,
    kTaki              = 104,
    kAPIX              = 105,
    kFortis            = 106
  } ;

  //! Readout mode
  /*! This enumeration type is used to identify a readout mode
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */
  enum EUTelReadoutMode {
    kRAW2             =   0,
    kRAW3             =   1,
    kZS               = 100,
    kZS2              = 101
  };


  //! Event type enum
  /*! This enumeration type has been introduced in order to
   *  distinguish the three main different kind of events available in
   *  a EUTelEventImpl. Those are the following:
   *
   *  \li <b>kBORE</b>: Beginning Of Run Event. This is a dummy event
   *  containing none of the real data collection. For the time being
   *  it is not used since the first event of the run can be
   *  identified precisely being the first after the run header
   *  record.
   *
   *  \li <b>kEORE</b>: End of Run Event. This is the last event of
   *  the run. If the DAQ software was able to terminate properly the
   *  run this event should be the last, otherwise it has to be
   *  appended "manually" using a sort of LCIO file fix.
   *
   *  \li <b>kDE</b>: Data Event. This is flag is used for all the
   *  other events in the run, the ones containing the data collection
   *  to be used for the analysis.
   *
   *  \li <b>kUNKNOWN</b>: type unknown or not set. This value has
   *  been introduced in order to extend the compatibility to non
   *  EUTelEventImpl event. In such case, in fact, asking for a not
   *  existing parameter will return 0.
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */
  enum EventType {
    kUNKNOWN  = 0,
    kBORE     = 1,
    kDE       = 2,
    kEORE     = 3
  };



  //! Cluster quality enum
  /*! This enum can be attached to a LCIO class describing a cluster
   *  or it can be inserted into the CellID describing the cluster
   *  collection. It is a five bit flag, that can be used to
   *  discriminate among different cluster qualities. This is because
   *  not all clusters passing the required cuts can be considered to
   *  be at the same quality levels. For example there are clusters
   *  centered on or so close to the detector edge that are
   *  incomplete. Those, even if they are passing the threshold for
   *  cluster identification they cannot be used for resolution
   *  studies, because their calculated position if biased by the
   *  missing pixels. The same apply for pixels with one missing pixel
   *  because was flagged as bad during the clustering processor.
   *
   *  Here a description of all allowed value of cluster quality and
   *  their meaning:
   *
   *  \li <b>kGoodCluster</b>: this flag is used to identify clusters
   *  having no problem at all.
   *
   *  \li <b>kIncompleteCluster</b>: this flag is used to identify
   *  clusters in which at least one pixel is missing because it was
   *  flagged as bad during the previous analysis processors.
   *
   *  \li <b>kBorderCluster</b>: this flag is used with clusters found
   *  to close to the detector edge and for that reason, their
   *  completeness cannot be garanted.
   *
   *  \li <b>kMergedCluster</b>: this flag is used to label clusters
   *  that have been tracked in a position so close to another cluster
   *  that charge sharing between the two cannot be excluded.
   *
   *  There are still two "not assigned" bits that can be used in the
   *  future to mark other different kind of bad quality clusters.
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */

  enum ClusterQuality {
    kGoodCluster       = 0,
    kIncompleteCluster = 1L << 0,
    kBorderCluster     = 1L << 1,
    kMergedCluster     = 1L << 2
  };

  //! Cluster quality bit-wise AND operator
  /*! This is a convenience operator used to identify the reason of a
   *  non good quality clusters. Bad quality clusters may be so for more
   *  than one reason simultaneously. This operator is used in the
   *  identification of such reasons.
   *
   *  @param a A cluster quality value
   *  @param b Another cluster quality value
   *  @return the bit wise and among @a a and @a b
   */
  ClusterQuality operator&(ClusterQuality a, ClusterQuality b);


  //! Cluster quality bit-wise OR operator
  /*! This is a crucial operator for ClusterQuality since, during the
   *  cluster search processor, a cluster maybe flagged with one or
   *  more than one "bad" qualities. For this reason, using this
   *  operator can allow to flag the same cluster with more than one
   *  bad qualities.
   *
   *  @param a A cluster quality value
   *  @param b Another cluster quality value
   *  @return the bit wise or among @a a and @a b
   */
  ClusterQuality operator|(ClusterQuality a, ClusterQuality b);


  //! Cluster quality self bit-wise OR operator
  /*! @see operator|(ClusterQuality a, ClusterQuality b)
   *  @bug Not working!!!! Have a look on the web...
   */
  ClusterQuality operator|=(ClusterQuality a, ClusterQuality b);

  //! Cluster quality operator <<
  /*! This operator can be used to stream out the value of a cluster
   *  quality enum. Both the numerical and the textual values are shown.
   *
   *  @param os The input output stream
   *  @param quality The variable to the be stream out
   *  @return The input output stream
   */
  std::ostream& operator<<(std::ostream& os, const ClusterQuality& quality);

  //! Cluster type enum
  /*! This enum is used in the encoding of a TrackerPulse to describe
   *  which was the underlying class used for the description of the
   *  cluster during the clusterization process itself.
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */
  enum ClusterType {
    kEUTelFFClusterImpl       = 0,
    kEUTelSparseClusterImpl   = 1,
    kEUTelSparseCluster2Impl  = 2,
    kEUTelDFFClusterImpl      = 3,
    kEUTelBrickedClusterImpl  = 4,
    kEUTelAPIXClusterImpl     = 5,
    // add here all the other cluster type numbering them in between 0
    // and 31 unknown
    kUnknown                  = 31
  };

  //! Sparse pixel type enum
  /*! This enumerator is used to define the sparsified pixel type.
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTELESCOPE.h,v 1.32 2009-08-01 21:05:03 jbehr Exp $
   */
  enum SparsePixelType {
    kEUTelBaseSparsePixel   = 0,
    kEUTelSimpleSparsePixel = 1,
    kEUTelAPIXSparsePixel   = 2,
    // add here your implementation
    kUnknownPixelType       = 31
  };

  //! SparsePixelType operator <<
  /*! This operator can be used to stream out the value of a sparse
   *  pixel type enum. Both the numerical and the textual values are
   *  shown.
   *
   *  @param os The input output stream
   *  @param type The variable to the be stream out
   *  @return The input output stream
   */
  std::ostream& operator<<(std::ostream& os, const SparsePixelType& type) ;



  // From here down a list of interesing utilities. Those are mainly
  // copied from the eudaq library and in case Eutelescope is built
  // w/o eudaq, than they are reimplemented here

  //! Convert to upper case
  /*! This function converts the input string into a full capital
   *  string.
   *
   *  @param inputString The string to be transformed in capitol
   *  letters.
   *  @return A fully capitalized string
   */
  std::string ucase(const std::string& inputString) ;

  //! Convert to lower case
  /*! This function takes a string as an input and return the same
   *  string written with all small letters
   *
   *  @param inputString The string to be transformed
   *  @return A string with only small letters
   */
  std::string lcase(const std::string& inputString);

  //! Trim a string
  /*! This function removes the leading and trailing white spaces from
   *  a string
   *
   *  @param inputString The string to be trimmed
   *  @return The trimmed string
   */
  std::string trim(const std::string& inputString);

  //! Escape a string
  /*! This function replaces all the characters that need to be
   *  escaped with the correct escape sequence.
   *
   *  @param inputString The string to be escaped
   *  @return The escaped string
   */
  std::string escape(const std::string& inputString);

  //! Get the first line of a text
  /*! This function returns the first line of a string. If the input
   *  string is made by only one line then the return string will be
   *  identical to the input one.
   *
   *  @param inputString The input text
   *  @return The first line of the input text
   */
  std::string firstline(const std::string& inputString);

  //! Tokenize a string according to a delimiter
  /*! This function takes an input string and returns a vector of
   *  strings with the input string chopped in pieces according to the
   *  defined delimiter
   *
   *  @param inputString The string to be split
   *  @param delim The token delimiter
   *  @return A vector of string
   */
  std::vector<std::string > split(const std::string& inputString, const std::string& delim);


  //! Convert to string
  /*! This very useful function converts any type to a string. There
   *  must be a compatible streamer defined for this function to
   *  work.
   *
   *  @param x The value to be converted
   *  @param digits The number of digits the output string has to have
   *  @return A string containing the input value
   */
  template <typename T>
  std::string to_string(const T& x, int digits = 0)
  {

#ifdef USE_EUDAQ

    return eudaq::to_string(x, digits);

#else

    std::ostringstream s;
    s << std::setfill('0') << std::setw(digits) << x;
    return s.str();

#endif
  }


  //! Converts any type to a string in hexadecimal
  /*! The input type needs to have a valid ostream streamer.
   *
   *  @param x The value to be converted
   *  @param digits The miimum number of digits, shorter numbers are
   *  padded with zeroes
   *
   *  @return A string representing the input value in hex
   */
  template< typename T>
  inline std::string to_hex(const T& x, int digits = 0)
  {

#ifdef USE_EUDAQ

    return eudaq::to_hex(x, digits);

#else
    std::ostringstream s;
    s << std::hex << std::setfill('0') << std::setw(digits) << x;
    return s.str();

#endif
  }

  //! Template specialization for T = const unsigned char
  template <>
  inline std::string to_hex(const unsigned char& x, int digits)
  {

#ifdef USE_EUDAQ

    return eudaq::to_hex(x, digits);

#else

    return to_hex((int) x, digits);

#endif
  }

  //! Template specialization for T = const signed char
  template <>
  inline std::string to_hex(const signed char& x, int digits)
  {

#ifdef USE_EUDAQ

    return eudaq::to_hex(x, digits);

#else

    return to_hex((int) x, digits);

#endif
  }

  //! Template specialization for T = const char
  template <>
  inline std::string to_hex(const char& x, int digits)
  {

#ifdef USE_EUDAQ

    return eudaq::to_hex(x, digits);

#else

    return to_hex((unsigned char) x, digits);

#endif
  }

  //! Converts a string to any type
  /*! This template function is particularly useful when getting
   *  parameters from GetTag
   *
   *  @param x The string to be converted
   *  @param def The default value to be used in case of an invalid
   *  string. This is also very useful to specify the proper template
   *  type w/o having to specify it explicitly
   *
   *  @return An object of type T with the value represented in x, or
   *  it that is not valid then the value of def.
   */
  template <typename T>
  inline T from_string(const std::string& x, const T& def = 0)
  {

#ifdef USE_EUDAQ

    return eudaq::from_string(x, def);

#else

    if (x == "") return def;
    T ret = def;
    std::istringstream s(x);
    s >> ret;
    char remain = '\0';
    s >> remain;
    if (remain) throw std::invalid_argument("Invalid argument: " + x);
    return ret;
#endif
  }

  //! Template specialization if T is a string
  template <>
  inline std::string from_string(const std::string& x, const std::string& def)
  {

#ifdef USE_EUDAQ

    return eudaq::from_string(x, def);

#else

    return x == "" ? def : x;

#endif
  }

}

#if USE_MARLIN
// custom verbosity levels
namespace streamlog {

  //! Verbosity level for the EUTelTestFitter processor
  DEFINE_STREAMLOG_LEVEL(TESTFITTERMESSAGE,  "TESTFITTERMESSAGE", message_base_level - 50 + 3 , STREAMLOG_MESSAGE_ACTIVE)

  //! Verbosity level for the EUTelMille processor
  DEFINE_STREAMLOG_LEVEL(MILLEMESSAGE,  "MILLEMESSAGE", message_base_level - 50 + 1 , STREAMLOG_MESSAGE_ACTIVE)

}
#endif

#endif
