#pragma once
#include <map>
#include <vector>
#include <utility>


struct ExpRun {
  int exp, run;
  ExpRun(int Exp, int Run) : exp(Exp), run(Run) {}
};

inline bool operator!=(ExpRun a, ExpRun b) { return (a.exp != b.exp || a.run != b.run); }
inline bool operator<(ExpRun a, ExpRun b) {return (1e6 * a.exp + a.run) < (1e6 * b.exp + b.run);}

std::map<ExpRun, std::pair<double, double>> filter(const std::map<ExpRun, std::pair<double, double>>& runs, double cut,
                                                   std::map<ExpRun, std::pair<double, double>>& runsRemoved);

class Splitter {
public:
  double tBest; // best time in hours
  double gapPenalty; // in relative units (higher -> less gaps)

  std::vector<std::pair<double, std::vector<int>>> cache;

  static std::map<ExpRun, std::pair<double, double>> mergeIntervals(std::map<ExpRun, std::pair<double, double>> I1,
                                                  std::map<ExpRun, std::pair<double, double>> I2);

  static std::vector<std::pair<double, double>> splitToSmall(std::map<ExpRun, std::pair<double, double>> runs,
                                                             double intSize = 1. / 60);

  std::vector<int> dynamicBreaks(std::vector<std::pair<double, double>>  runs);

  double getMinLoss(const std::vector<std::pair<double, double>>&  vec,   int b, std::vector<int>& breaks);
  double lossFunction(const std::vector<std::pair<double, double>>&  vec, int s, int e);

  std::vector<std::vector<std::map<ExpRun, std::pair<double, double>>>>  getIntervals(std::map<ExpRun, std::pair<double, double>>
      runs, double tBestSize, double tBestVtx, double GapPenalty);

  static std::pair<double, double> getStartEnd(std::vector<std::map<ExpRun, std::pair<double, double>>> res)
  {
    return {res.front().begin()->second.first,
            res.back().rbegin()->second.second};
  }

  static std::vector<double> getBreaks(std::vector<std::map<ExpRun, std::pair<double, double>>> res)
  {
    std::vector<double> breaks;
    for (int k = 0; k < int(res.size()) - 1; ++k) {
      double e = res.at(k).rbegin()->second.second; //end of the previous interval
      double s = res.at(k + 1).begin()->second.first; //start of the next interval
      breaks.push_back((e + s) / 2.);
    }
    return breaks;
  }


};

