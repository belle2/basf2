/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERMLPDATA_H
#define CDCTRIGGERMLPDATA_H

#include <framework/logging/Logger.h>
#include <iostream>
#include <fstream>
#include <TObject.h>
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/multi_array.hpp"
#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {
  /** Struct for training data of a single MLP for the neuro trigger */
  class CDCTriggerMLPData : public TObject {
  public:
    struct HeaderSet {
      float relID[18];
      unsigned exPert;
      HeaderSet() {}
      HeaderSet(std::vector<float> line)
      {
        if (line.size() == 19) {
          exPert = line[0];
          for (unsigned i = 1; i <= 19; i++) {
            relID[i] = line[i];
          }
        } else {B2ERROR("ERROR! wrong lengthof relID vector!");}
      }
      void operator= (std::vector<float> line)
      {
        if (line.size() == 19) {
          exPert = line[0];
          for (unsigned i = 1; i <= 19; i++) {
            relID[i] = line[i];
          }
        } else {B2ERROR("ERROR! wrong length of relID vector!");}


      }
      HeaderSet(unsigned expert, std::vector<float> relevantID)
      {
        if (relevantID.size() == 18) {
          for (unsigned i = 0; i < 18; ++i) {relID[i] = relevantID[i];}
        } else {B2ERROR("ERROR! wrong length of relID vector!");}
        exPert = expert;
      }
      friend std::ostream& operator << (std::ostream& out, const HeaderSet& hset)
      {
        out << hset.exPert << '\t';
        for (unsigned i = 0; i < 18; ++i) { out << hset.relID[i] << '\t';}
        return out;
      }
      friend std::istream& operator >> (std::istream& in, HeaderSet& hset)
      {
        std::string help;
        std::string helpline;
        help = "";
        helpline = "";
        std::getline(in, helpline, '\n');
        if (helpline.length() < 2) {return in;}
        std::stringstream ss(helpline);
        std::getline(ss, help, '\t');

        hset.exPert = std::stoul(help);
        for (unsigned i = 0; i < 18; ++i) {
          help = "";
          std::getline(ss, help, '\t');
          hset.relID[i] = std::stof(help);
        }
        return in;
      }


    };
    /** Struct to keep one set of training data for either training, validation or testing.
     * Having this struct makes it easier to save the data to an output filestream.
     */
    template <const unsigned inLen, const unsigned outLen>
    struct NeuroSet {
      float input[inLen];
      float target[outLen];
      int exp;
      int run;
      int subrun;
      int evt;
      int track;
      int ntracks;
      float nnscalez;
      float nnscaletheta;
      int nnrawz;
      int nnrawtheta;
      float inphi;
      float intheta;
      unsigned expert;
      float ininvpt;
      std::string headline;
      NeuroSet()
      {
        headline = "Experiment\tRun\tSubrun\tEvent\tTrack\tnTracks\tExpert\tiNodes\toNodes\t";
        for (unsigned i = 0; i < inLen / 3; ++i) {
          headline += "SL" + std::to_string(i) + "-relID\t";
          headline += "SL" + std::to_string(i) + "-driftT\t";
          headline += "SL" + std::to_string(i) + "-alpha\t";
        }
        headline += "RecoZ\tRecoTheta\tScaleZ\tRawZ\tScaleTheta\tRawTheta\t2DPhi\t3DTheta\t2DinvPt\n";
      }

      NeuroSet(float xin[inLen], float xout[outLen],  int xexp, int xrun, int xsubrun, int xevt, int xtrack, unsigned xexpert,
               int xntracks, int xnnrawz, int xnnrawtheta, float xnnscalez, float xnnscaletheta, float xinphi, float xintheta, float xininvpt)
      {
        for (unsigned i = 0; i < inLen; ++i) {
          input[i] = xin[i];
        }
        for (unsigned i = 0; i < outLen; ++i) {
          target[i] = xout[i];
        }
        exp = xexp;
        run = xrun;
        subrun = xsubrun;
        evt = xevt;
        track = xtrack;
        expert = xexpert;

        ntracks = xntracks;
        nnscalez = xnnscalez;
        nnscaletheta = xnnscaletheta;
        nnrawz = xnnrawz;
        nnrawtheta = xnnrawtheta;
        inphi = xinphi;
        intheta = xintheta;
        ininvpt = xininvpt;
        headline = "Experiment\tRun\tSubrun\tEvent\tTrack\tnTracks\tExpert\tiNodes\toNodes\t";
        for (unsigned i = 0; i < inLen / 3; ++i) {
          headline += "SL" + std::to_string(i) + "-relID\t";
          headline += "SL" + std::to_string(i) + "-driftT\t";
          headline += "SL" + std::to_string(i) + "-alpha\t";
        }
        headline += "RecoZ\tRecoTheta\tScaleZ\tRawZ\tScaleTheta\tRawTheta\t2DPhi\t3DTheta\t2DinvPt\n";

      }
      friend std::ostream& operator << (std::ostream& out, const NeuroSet& dset)
      {
        out << dset.exp << '\t' << dset.run << '\t' << dset.subrun << '\t' << dset.evt << '\t' << dset.track << '\t' << dset.ntracks << '\t'
            << dset.expert << '\t'
            << inLen << '\t' << outLen << '\t';
        for (auto indata : dset.input) {out << indata << '\t';}
        for (auto outdata : dset.target) {out << outdata << '\t';}
        out << dset.nnscalez << '\t' << dset.nnrawz << '\t' << dset.nnscaletheta << '\t' << dset.nnrawtheta << '\t' << dset.inphi << '\t' <<
            dset.intheta << '\t' << dset.ininvpt << '\t';
        return out;
      }
      friend std::istream& operator >> (std::istream& in, NeuroSet& dset)
      {
        std::string help;
        help = "";
        std::getline(in, help, '\t');
        if (help.length() < 2) {return in;}
        dset.exp = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.run = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.subrun = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.evt = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.track = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.ntracks = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.expert = std::stoul(help);
        // check if input and target size match:
        std::string insize = "";
        std::string outsize = "";
        std::getline(in, insize, '\t');
        if (std::stoul(insize) != inLen) {
          B2ERROR("Input and output format of neurotrigger training data does not  match!");
        }
        std::getline(in, outsize, '\t');
        if (std::stoul(outsize) != outLen) {
          B2ERROR("Input and output format of neurotrigger training data does not  match!");
        }
        for (unsigned i = 0; i < inLen; ++i) {
          help = "";
          std::getline(in, help, '\t');
          try {
            dset.input[i] = std::stof(help);
          } catch (const std::out_of_range& oor) {
            B2WARNING("out of range error: " << help << " is not in float range!");
            dset.input[i] = 0.;
          }
        }
        for (unsigned i = 0; i < outLen; ++i) {
          help = "";
          std::getline(in, help, '\t');
          try {
            dset.target[i] = std::stof(help);
          } catch (const std::out_of_range& oor) {
            B2WARNING("out of range error: " << help << " is not in float range!");
            dset.target[i] = 0.;
          }
        }
        help = "";
        std::getline(in, help, '\t');
        dset.nnscalez = std::stof(help);
        help = "";
        std::getline(in, help, '\t');
        dset.nnrawz = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.nnscaletheta = std::stof(help);
        help = "";
        std::getline(in, help, '\t');
        dset.nnrawtheta = std::stoi(help);
        help = "";
        std::getline(in, help, '\t');
        dset.inphi = std::stof(help);
        help = "";
        std::getline(in, help, '\t');
        dset.intheta = std::stof(help);
        help = "";
        std::getline(in, help, '\t');
        dset.ininvpt = std::stof(help);
        help = "";
        return in;
      }
    };

    /** default constructor. */
    CDCTriggerMLPData(): inputSamples(), targetSamples(), hitCounters(), trackCounter(0) { }
    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerMLPData() { }

    /** add hit counters for a layer with nWires */
    void addCounters(unsigned nWires)
    {
      std::vector<unsigned short> counters;
      counters.assign(nWires, 0);
      hitCounters.push_back(counters);
    }
    /** increase counter for super layer and track segment number in super layer.
     *  track segment number can be negative.
     *  hits in the wrong hemisphere (not in [-nWires/4, nWires/4]) are skipped. */
    void addHit(unsigned iSL, int iTS);
    /** increase track counter */
    void countTrack() { ++trackCounter; }
    template<unsigned inLen, unsigned outLen>
    void addSample(const NeuroSet<inLen, outLen>& dsample)
    {
      std::vector<float> in(dsample.input, dsample.input + sizeof dsample.input / sizeof dsample.input[0]);
      inputSamples.push_back(in);
      std::vector<float> out(dsample.target, dsample.target + sizeof dsample.target / sizeof dsample.target[0]);
      targetSamples.push_back(out);
      explist.push_back(dsample.exp);
      runlist.push_back(dsample.run);
      subrunlist.push_back(dsample.subrun);
      evtlist.push_back(dsample.evt);
      tracklist.push_back(dsample.track);

    }

    /** add a pair of input and target */
    void addSample(const std::vector<float>& input, const std::vector<float>& target, const int& expnumber, const int& runnumber,
                   const int& subrunnumber, const int& eventnumber, const int& tracknumber)
    {
      inputSamples.push_back(input);
      targetSamples.push_back(target);
      explist.push_back(expnumber);
      runlist.push_back(runnumber);
      subrunlist.push_back(subrunnumber);
      evtlist.push_back(eventnumber);
      tracklist.push_back(tracknumber);
    }

    /** get track counter */
    short getTrackCounter() const { return trackCounter; }
    /** get hit counter for super layer and track segment number is super layer.
     *  track segment number can be negative.
     *  @return counter or 0 (for invalid input) */
    unsigned short getHitCounter(unsigned iSL, int iTS) const;
    /** get number of samples (same for input and target) */
    unsigned nSamples() const { return targetSamples.size(); }
    /** get input vector of sample i */
    const std::vector<float>& getInput(unsigned i) const { return inputSamples[i]; }
    /** get target value of sample i */
    const std::vector<float>& getTarget(unsigned i) const { return targetSamples[i]; }
    const std::vector<int>& getevtList() const { return getEvtList(); }
    const std::vector<int>& getEvtList() const { return evtlist; }
    const std::vector<int>& getExpList() const { return explist; }
    const std::vector<int>& getRunList() const { return runlist; }
    const std::vector<int>& getSubRunList() const { return subrunlist; }
    const std::vector<int>& getTrackList() const { return tracklist; }
    const int& getevtnum(unsigned i) const { return evtlist[i]; }
  private:
    /** list of input vectors for network training. */
    std::vector<std::vector<float>> inputSamples;
    /** list of target values for network training. */
    std::vector<std::vector<float>> targetSamples;
    /** hit counter of active track segment IDs,
     *  used to determine the relevant id range for an MLP. */
    std::vector<std::vector<unsigned short>> hitCounters;
    /** number of tracks used for the hit counter. */
    short trackCounter;
    /** Vectors for experiment, run, event and tracknumber */
    std::vector<int> explist;
    std::vector<int> runlist;
    std::vector<int> subrunlist;
    /** List for debug purposes to store the event number for every entry of input/target vector */
    std::vector<int> evtlist;
    std::vector<int> tracklist;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerMLPData, 2);
  };
}
#endif
