#include <TRandom.h>
#include <vector>
//index prescalefactor cut
//0 1 n_2d_finder>=3
//1 1 n_2d_finder==2&&BhabhaVeto==0
//2 1 n_2d_finder>=1&&n_gc>=1&&BhabahVeto==0&&SBhabhaVeto==0
//3 1 n_c>=3&&n_high_c1>=1&&eclBhabhaVeto==0
//4 1 n_c>=2&&n_high_c4e==0&&n_bbc>=1&&eclBhabhaVeto==0
//5 1 n_high_c2>=1&&eclBhabhaVeto==0
//6 2000 n_c>=2&&n_bbc>=1
//7 2000 n_2d_finder>=1&&n_gc>=1
//8 2000 n_high_c2>=1
//9 1000 eclbhabha
//10 1000 n_2d_finder>=1&&n_high_c3>=1&&n_bbtc>=1
//11 1000 n_high_c3>=1&&n_2d_finder==0

//GRL Objects
//0 n_2d_finder
//1 n_c
//2 n_high_c1
//3 n_high_c2b
//4 n_high_c3
//5 n_high_c4e
//6 n_gc
//7 n_bbc
//8 n_bbtc
//9 BhabhaVeto
//10 SBhabhaVeto
//11 eclBhabhaVeto

namespace Belle2 {

  /**set the total number of trigger lines and prescalefactor*/
  const int ntrgline = 18;
  /**prescale factor*/
  double scalef_phase2[ntrgline] = {1, 1, 20, 1, 1, 1, 1, 1, 1, 1, 1, 10, 10, 1, 1, 1, 1, 1};
  double scalef_phase3[ntrgline] = {1, 1, 20, 2, 1, 1, 1, 1, 2, 1, 1, 20, 20, 1, 5, 1, 3, 5};

  /**select one event in number of prescale factor events*/
  int doprescale(int f)
  {

    int Val = 0;
    double ran = gRandom->Uniform(f);
    if (ceil(ran) == f) Val = 1;
    return Val;
  }

  /**do trigger with the import objects, and export the trigger results to trgres*/
  void dotrigger(std::vector<int>& trgres, std::vector<int> objects, std::string phase)
  {
    /**Trigger Logics are defined in TRGGDL!!!*/

    /*
    //line 0
        if (objects[0] >= 3) trgres.push_back(doprescale(scalef[0]));
        else trgres.push_back(0);
    //line 1
        if (objects[0] == 2 && objects[9] == 0) trgres.push_back(doprescale(scalef[1]));
        else trgres.push_back(0);
    //line 2
        if (objects[0] >= 1 && objects[6] >= 1 && objects[9] == 0 && objects[10] == 0)trgres.push_back(doprescale(scalef[2]));
        else trgres.push_back(0);
    //line 3
        if (objects[1] >= 3 && objects[2] >= 1 && objects[11] == 0) trgres.push_back(doprescale(scalef[3]));
        else trgres.push_back(0);
    //line 4
        if (objects[1] >= 2 && objects[5] == 0 && objects[7] >= 1 && objects[11] == 0) trgres.push_back(doprescale(scalef[4]));
        else trgres.push_back(0);
    //line 5
        if (objects[3] >= 1 && objects[11] == 0) trgres.push_back(doprescale(scalef[5]));
        else trgres.push_back(0);
    //line 6
        if (objects[1] >= 2 && objects[7] >= 1) trgres.push_back(doprescale(scalef[6]));
        else trgres.push_back(0);
    //line 7
        if (objects[0] >= 1 && objects[6] >= 1) trgres.push_back(doprescale(scalef[7]));
        else trgres.push_back(0);
    //line 8
        if (objects[3] >= 1) trgres.push_back(doprescale(scalef[8]));
        else trgres.push_back(0);
    //line 9
        if (objects[11] == 1) trgres.push_back(doprescale(scalef[9]));
        else trgres.push_back(0);
    //line 10
        if (objects[0] >= 1 && objects[4] >= 1 && objects[8] >= 1) trgres.push_back(doprescale(scalef[11]));
        else trgres.push_back(0);
    //line 11
        if (objects[0] == 0 && objects[4] >= 1) trgres.push_back(doprescale(scalef[11]));
        else trgres.push_back(0);
    //add new trigger line by users here, e.g.
        //if(objects[0]==0)trgres.push_back(doprescale(1));
        //else trgres.push_back(0);
    */
  }

}

