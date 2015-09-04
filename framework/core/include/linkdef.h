#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CalcMeanCov<2, float>+; //implicit
#pragma link C++ class Belle2::CalcMeanCov<2, double>+; //implicit
#pragma link C++ class Belle2::ModuleStatistics+;
#pragma link C++ class vector<Belle2::ModuleStatistics>+;
#pragma link C++ class Belle2::ProcessStatistics+;
#pragma link C++ class Belle2::Environment+;
#pragma link C++ class Belle2::RandomGenerator+;

//avoid warning when reading m_stats written by root 5
//(which is saved with wrong type Belle2::ModuleStatistics::CalcMeanCov<2,value_type>)
//NOTE: It still cannot be read, and the resulting objects will be empty.
#pragma read sourceClass="Belle2::ModuleStatistics" \
  source="Belle2::CalcMeanCov<2, double> m_stats[6]" \
  targetClass="Belle2::ModuleStatistics" target="m_stats" \
  code="{ \
    m_stats[0] = onfile.m_stats[0]; \
    m_stats[1] = onfile.m_stats[1]; \
    m_stats[2] = onfile.m_stats[2]; \
    m_stats[3] = onfile.m_stats[3]; \
    m_stats[4] = onfile.m_stats[4]; \
    m_stats[5] = onfile.m_stats[5]; \
  }"

#endif
