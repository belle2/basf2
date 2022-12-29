/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <cmath>
#include <iostream>
#include <TObject.h>
#include <TCanvas.h>
#include <TH1F.h>

namespace Belle2 {

  /**
   *   dE/dx injection time calibration constants
   */

  class CDCDedxInjectTime: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxInjectTime(): m_injectionvar() {};

    /**
     * Constructor
     */
    CDCDedxInjectTime(const std::vector<std::vector<double>>& vinjcalib): m_injectionvar(vinjcalib) {};

    /**
     * Destructor
     */
    ~CDCDedxInjectTime() {};


    /** Return dE/dx mean or norm-reso value for the given time and ring
     * @param svar is option for printing mean and reso calibration
     * @param sfx to add suffix in file save
     */
    void printCorrection(std::string svar, std::string sfx) const
    {

      if (svar != "mean" && svar != "reso") {
        B2WARNING("wrong variable input, choose mean or reso");
        return;
      }

      std::string sring[2] = {"ler", "her"};
      TCanvas* cdraw = new TCanvas("cdraw", "cdraw", 900, 500);
      cdraw->cd();
      cdraw->SetGridy(1);

      for (int ir = 0; ir < 2; ir++) {

        unsigned int iv = ir * 3 + 1 ;
        if (svar == "reso") iv = ir * 3 + 2 ;

        unsigned int sizev = m_injectionvar[iv].size(); //mean or reso
        unsigned int sizet = m_injectionvar[ir * 3].size(); //time
        if (sizev == 0 || sizet == 0) {
          B2ERROR("empty calibration vector");
          break;
        }

        unsigned int tedges[sizet]; //time edges array
        std::copy(m_injectionvar[ir * 3].begin(), m_injectionvar[ir * 3].end(), tedges);

        std::string hname = Form("hconst_%s_%s_%s", svar.data(), sring[ir].data(), sfx.data());
        std::string title = Form("%s corrections;injection-time (#mu-sec);%s", svar.data(), svar.data());
        TH1F hconst(hname.data(), title.data(), sizet - 1, 0, sizet - 1);

        for (unsigned int ib = 0; ib < sizev; ib++) {

          double corr = m_injectionvar[iv].at(ib);
          double ledge = tedges[ib];
          double uedge = tedges[ib + 1];

          std::string label;
          if (ledge < 2e4)label = Form("%0.01f-%0.01fK", ledge / 1e3, uedge / 1e3);
          else if (ledge < 1e5)label = Form("%0.00f-%0.00fK", ledge / 1e3, uedge / 1e3);
          else label = Form("%0.01f-%0.01fM", ledge / 1e6, uedge / 1e6);

          hconst.SetBinContent(ib + 1, corr);
          hconst.SetBinError(ib + 1, corr * 0.001);
          hconst.GetXaxis()->SetBinLabel(ib + 1, Form("%s", label.data()));
          B2INFO("ring: " << sring[ir] << ", time (" << ledge << "-" << uedge << "), mean: " << corr << "");
        }

        hconst.SetStats(0);
        hconst.SetMarkerColor(ir + 1);
        hconst.SetMarkerSize(1.05);
        hconst.SetMarkerStyle(24);
        hconst.GetXaxis()->SetLabelOffset(-0.055);
        hconst.GetYaxis()->SetRangeUser(0.60, 1.10);
        if (svar == "reso")hconst.GetYaxis()->SetRangeUser(0.01, 0.20);

        if (ir == 0)hconst.DrawCopy("");
        else hconst.DrawCopy(" same");
      }

      cdraw->SaveAs(Form("cdcdedx_timeinject_const_%s_%s.pdf", svar.data(), sfx.data()));
      delete cdraw;

    };

    /** Return vector of all constant vector of payload
     *
     */
    std::vector<std::vector<double>> getConstVector() const
    {
      //std::vector<std::vector<double>> &vector
      return m_injectionvar;
    };

    /** Return time vector
     * @param ring is injection ring number (0/1 for LER/HER)
     */
    std::vector<double> getTimeVector(const unsigned int ring) const
    {
      if (ring > 2) B2ERROR("wrong index for injection ring ");
      return m_injectionvar[ring * 3];
    };

    /** Return dedx mean vector
     * @param ring is injection ring number (0/1 for LER/HER)
     */
    std::vector<double> getMeanVector(const int ring) const
    {
      if (ring <= 0 || ring > 2) B2ERROR("wrong index for injection ring ");
      return m_injectionvar[ring * 3 + 1];
    };

    /** Return dedx reso vector
     * @param ring is injection ring number (0/1 for LER/HER)
     */
    std::vector<double> getResoVector(const unsigned int ring) const
    {
      if (ring > 2) B2ERROR("wrong index for injection ring ");
      return m_injectionvar[ring * 3 + 2];
    };

    /** Return time bin for the given time array
    * @param n length of time array
    * @param array of time
    * @param value of input time
    */
    unsigned int getTimeBin(unsigned int n, unsigned int* array, unsigned int value) const
    {
      int nabove, nbelow, middle;
      nabove = n + 1;
      nbelow = 0;
      while (nabove - nbelow > 1) {
        middle = (nabove + nbelow) / 2;
        if (value == array[middle - 1]) return middle - 1;
        if (value  < array[middle - 1]) nabove = middle;
        else nbelow = middle;
      }
      return nbelow - 1;
    }

    /** Return dE/dx mean or norm-reso value for the given time and ring
    * @param svar is option for mean and reso calibration factor
    * @param ring is injection ring number (0/1 for LER/HER)
    * @param time is injection time (large 0-20sec range)
    */
    double getCorrection(std::string svar, unsigned int ring, unsigned int time) const
    {
      if (svar != "mean" && svar != "reso") {
        B2ERROR("wrong var input, choose mean or reso");
        return 1.0;
      }

      if (std::isnan(ring) || std::isnan(time))return 1.0;

      if (ring > 2) {
        B2ERROR("wrong ring input, choose 0 or 1");
        return 1.0;
      }

      unsigned int iv = ring * 3 + 1 ;
      if (svar == "reso") iv = ring * 3 + 2 ;

      unsigned int sizev = m_injectionvar[iv].size(); //mean or reso
      unsigned int sizet = m_injectionvar[ring * 3].size(); //time
      if (sizet == 0 || sizev == 0) {
        B2ERROR("calibration vectors are empty");
        return 1.0;
      }

      unsigned int tedges[sizet]; //time edges array
      std::copy(m_injectionvar[ring * 3].begin(), m_injectionvar[ring * 3].end(), tedges);

      if (time >= 5e6)time = 5e6 - 10;

      unsigned int it = getTimeBin(sizet, tedges, time);
      double center = 0.5 * (m_injectionvar[ring * 3].at(it) + m_injectionvar[ring * 3].at(it + 1));

      //no corr before veto bin (usually one or two starting bin)
      //intrapolation for entire range except
      //--extrapolation (for first half and last half of intended bin)
      int thisbin = it, nextbin = it;
      if (center != time && it > 0) {

        if (time < center) {
          thisbin = it - 1;
        } else {
          if (it < sizet - 2)nextbin = it + 1;
          else thisbin = it - 1;
        }

        if (it <= 2) {
          double diff = m_injectionvar[iv].at(2) - m_injectionvar[iv].at(1) ;
          if (diff < -0.015) { //difference above 1.5%
            thisbin = it;
            if (it == 1) nextbin = it;
            else nextbin = it + 1;
          } else {
            if (it == 1) {
              thisbin = it;
              nextbin = it + 1;
            }
          }
        }
      }

      double thisdedx = m_injectionvar[iv].at(thisbin);
      double nextdedx = m_injectionvar[iv].at(nextbin);

      double thistime = 0.5 * (m_injectionvar[ring * 3].at(thisbin) + m_injectionvar[ring * 3].at(thisbin + 1));
      double nexttime = 0.5 * (m_injectionvar[ring * 3].at(nextbin) + m_injectionvar[ring * 3].at(nextbin + 1));

      double newdedx = m_injectionvar[iv].at(it);
      if (thisbin != nextbin)
        newdedx = thisdedx + ((nextdedx - thisdedx) / (nexttime - thistime)) * (time - thistime);

      return newdedx;
    };


  private:

    /** CDC dE/dx injection time payloads for dEdx mean and reso.
     * different for LER and HER
    */
    std::vector<std::vector<double>> m_injectionvar; /**< vector to store payload values*/
    ClassDef(CDCDedxInjectTime, 1); /**< ClassDef */

  };
} // end namespace Belle2
