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
     *Usefull contructor
     *@param template fit rising edge, lower 8 bit are fraction
     *@param background offset
     *@param amplitude
     *@param chi square
     */
    TOPTemplateFitResult(unsigned short risingEdge, short backgroundOffset,
                         short amplitude, unsigned short chisquare);

    /**
     * Sets background offset
     * @param background offset
     */
    void setBackgroundOffset(short backgroundOffset) { m_backgroundOffset = backgroundOffset;}

    /**
     * Sets amplitude
     * @param amplitude
     */
    void setAmplitude(short amplitude) {m_amplitude = amplitude;}

    /**
     * Sets chi square
     * @param chi square
     */
    void setChisquare(unsigned short chisquare) {m_chisquare = chisquare;}

    /**
     * Sets rising edge
     * @param rising edge
     */
    void setRisingEdge(unsigned short risingEdge);

    /**
     * Get rising Edge
     * @return rising edge
     */
    double getRisingEdge() const {return m_risingEdge;}

    /**
     * Get rising Edge Raw
     * @return rising edge raw
     */
    unsigned getRisingEdgeRaw() const {return m_risingEdgeRaw;}

    /**
     * Get background offset
     * @return background offset
     */
    short getBackgroundOffset() const {return m_backgroundOffset;}

    /**
     * Get ampltide
     * @return amplitude
     */
    short getAmplitude() const {return m_amplitude;};

    /**
     * Get chi square
     * @return chi square
     */
    unsigned short getChisquare() const {return m_chisquare;}

    /**
     * Get mean position of template function
     */
    double getMean() const;

    /**
     * Get value of template function
     * @param position to evaluate template function
     * @return value of template function
     */
    double getTemplateFunctionValue(double x) const;

  private:

    /**
     * Template is defined as crystal ball.
     * @param position to evaluate crystal ball function
     * @return value of crystal ball function
     */
    double crystalball(double x) const ;

    /**
     * Convert rising edge saved in short to double
     * @param input short
     * @return output double
     */
    double risingEdgeShortToRisingEdgeDouble(unsigned short risingEdgeS) const;

    double m_risingEdge;/**< template fit rising edge position*/
    unsigned m_risingEdgeRaw;/**< template fit rising edge position received from FEE*/
    short m_backgroundOffset;/**< background offset from fit*/
    short m_amplitude;/**< amplitude from fit*/
    unsigned short m_chisquare;/**< chi square value of template fit */
    const float m_templateSigma = 1.81;/**< template gaus sigma */
    const float m_templateAlpha = -0.45;/**< template exponential tail position */
    const float m_templateN = 18.06;/**< template exponential decay constant */

    ClassDef(TOPTemplateFitResult, 1); /**< ClassDef */
  };

}
