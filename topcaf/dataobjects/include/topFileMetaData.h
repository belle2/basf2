#ifndef TOPFILEMETADATA_H
#define TOPFILEMETADATA_H

#include <TObject.h>
#include <string>
#include <framework/logging/Logger.h>

namespace Belle2 {

  class topFileMetaData : public TObject {

  public:
    topFileMetaData() {}

    ~topFileMetaData() {}

    std::string getDir() {return m_dir;}
    std::string getFilename() {return m_filename;}
    std::string getExperiment() {return m_experiment;}
    std::string getRun() {return m_run;}
    std::string getRuntype() {return m_runtype;}

    void set(std::string dir, std::string filename);

  private:
    std::string m_dir;
    std::string m_filename;
    std::string m_experiment;
    std::string m_run;
    std::string m_runtype;

    ClassDef(topFileMetaData, 1);

  };

}

#endif
