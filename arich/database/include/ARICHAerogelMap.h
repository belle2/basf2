/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik, Manca Mrvar                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TTimeStamp.h>
#include <string>
class ARICHAerogelInfo;
namespace Belle2 {
  /**
  *   Map of the Aerogel placement
  */
  class ARICHAerogelMap: public TObject {
  public:
    /**
     * Various constants
     */
    enum {c_layers = 2, /**< number of layers */
         };

    /**
     * Default constructor
     */
    ARICHAerogelMap(): m_ring(0), m_column(0), m_timeStamp(0, 0, 0, kTRUE, 0) {};

    /**
     * Constructor
     */
    ARICHAerogelMap(int ring, int column, TTimeStamp timeStamp)
    {
      m_ring = ring;
      m_column = column;
      m_timeStamp = timeStamp;
    }

    /**
     * Destructor
     */
    ~ARICHAerogelMap() {};

    /** Return Aerogel Identifier
     * @return Aerogel Identifier
     */
    std::string getAerogelID();


    /**
      * Return Aerogel Ring Identifier
      * @return Aerogel Ring
      */
    int getAerogelRingID() const {return m_ring;}

    /**
     * Return Aerogel Column Identifier
     * @return Aerogel Column
     */
    int getAerogelColumnID() const {return m_column;}

    /**
     * Return Aerogel Layer Identifier
     * @return Aerogel Layer
     */
    unsigned int getAerogelLayer(unsigned i) const
    {
      i++;
      if (i < c_layers) {
        return m_layer[i];
      }
      return 0;
    }

    /** Return Aerogel installation date
     * @return Aerogel installation date
     */
    TTimeStamp getAerogelTimeStamp() const {return m_timeStamp; }

    /** Return comment
     * @return comment
     */
    std::string getAerogelMapComment();

    /** Set Aerogel Identifier
     * @param Aerogel Identifier
     */
    void setAerogelIdentifier(const std::string& agelid) {m_id = agelid; }

    /**
     * Set Aerogel Ring Identifier
     * @param Ring Identifier
     */
    void setAerogelRingID(int ring) {m_ring = ring;}

    /**
     * Set Aerogel Column Identifier
     * @param Column Identifier
     */
    void setAerogelColumnID(int column) {m_column = column;}

    /** Set Aerogel installation date
     *
     */
    TTimeStamp setAerogelTimeStamp();

    /** Set comment
     * @param comment
     */
    std::string setAerogelMapComment(const std::string& comment) {m_comment = comment; }





  private:
    std::string m_id;                     /**< Aerogel Identifier */
    int m_ring;                           /**< Aerogel Ring Identifier */
    int m_column;                         /**< Aerogel Column identifier */
    unsigned int m_layer[c_layers];       /**< Aerogel Layers */
    TTimeStamp m_timeStamp;               /**< Installation Date */
    std::string m_comment;                /**< optional comment */

    ClassDef(ARICHAerogelMap, 1);           /**< ClassDef */
  };
} // end namespace Belle2
