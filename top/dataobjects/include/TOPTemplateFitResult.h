/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Weber                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store template fit result from feature extraction data.
   */
  class TOPTemplateFitResult : public RelationsObject {
  public:

    /**
     *Default constructor
     */
    TOPTemplateFitResult();

    /**
     *Usefull contructor for data from Zynqs
     *@param template fit rising edge, lower 8 bit are fraction
     *@param background offset
     *@param amplitude
     *@param chi square
     */
    explicit TOPTemplateFitResult(short risingEdge, short backgroundOffset,
                                  short amplitude, short chisquare);

    /**
     *Usefull contructor for software implementation of template fit in basf2
     *@param template fit rising edge in samples
     *@param template fit rising edge in ns
     *@param background offset
     *@param amplitude
     *@param chi square
     */
    TOPTemplateFitResult(double risingEdge, double risingEdgeTime, double backgroundOffset,
                         double amplitude, double chisquare);

    /**
     * Sets background offset
     * @param background offset
     */
    void setBackgroundOffset(double backgroundOffset) { m_backgroundOffset = backgroundOffset;}

    /**
     * Sets amplitude
     * @param amplitude
     */
    void setAmplitude(double amplitude) {m_amplitude = amplitude;}

    /**
     * Sets chi square
     * @param chi square
     */
    void setChisquare(double chisquare) {m_chisquare = chisquare;}

    /**
     * Sets rising edge from Zynq and converts to double
     * This does not convert from sample to time space
     * @param rising edge
     */
    void setRisingEdgeAndConvert(unsigned short risingEdge);

    /**
     * Sets rising edge in samples
     * @param rising edge in samples
     */
    void setRisingEdge(double risingEdge) {m_risingEdge = risingEdge;}

    /**
     * Sets rising edge in ns
     * @param rising edge in ns
     */
    void setRisingEdgeTime(double risingEdge) {m_risingEdgeTime = risingEdge;}

    /**
     *Get rising edge in samples
     *@return rising edge in samples
     */
    double getRisingEdge() const {return m_risingEdge;}

    /**
     * Get rising Edge
     * @return rising edge in ns
     */
    double getRisingEdgeTime() const {return m_risingEdgeTime;}

    /**
     * Get rising Edge Raw
     * @return rising edge raw
     */
    unsigned getRisingEdgeRaw() const {return m_risingEdgeRaw;}

    /**
     * Get background offset
     * @return background offset
     */
    double getBackgroundOffset() const {return m_backgroundOffset;}

    /**
     * Get ampltide
     * @return amplitude
     */
    double getAmplitude() const {return m_amplitude;};

    /**
     * Get chi square
     * @return chi square
     */
    double getChisquare() const {return m_chisquare;}

  private:

    /**
     * Convert rising edge saved in short to double
     * @param input short
     * @return output double
     */
    double risingEdgeShortToRisingEdgeDouble(unsigned short risingEdgeS) const;

    double m_risingEdge;/**< template fit rising edge position*/
    double m_risingEdgeTime; /**<template fit rising edge in ns*/
    double m_backgroundOffset;/**< background offset from fit*/
    double m_amplitude;/**< amplitude from fit*/
    double m_chisquare;/**< chi square value of template fit */
    unsigned m_risingEdgeRaw;/**< template fit rising edge position received from FEE*/

    ClassDef(TOPTemplateFitResult, 2); /**< ClassDef */
  };

}
