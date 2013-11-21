#include "MonitorGenerator.h"

#include <base/StringUtil.h>
#include <base/ConfigFile.h>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <algorithm>
#include <string>

using namespace Belle2;

void MonitorGenerator::create()
{
  ConfigFile config("slc_config");
  std::cout << "making directory: " << _output_dir << std::endl;
  system(std::string(("mkdir -p ") + _output_dir).c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/include").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/src").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/tmp").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/lib").c_str());
  std::cout << "copying makefiles: " << _output_dir << std::endl;
  std::cout << "creating header file for manager class : "
            << createHeader() << std::endl;
  std::cout << "creating source file for manager class : "
            << createSource() << std::endl;
  std::cout << "creating source file for dynamic loader of histo manager : "
            << createLibSource() << std::endl;
  system(Belle2::form("echo \"USER_CLASS=%s\" > %s/Makefile ",
                      _name.c_str(), _output_dir.c_str()).c_str());
  system(Belle2::form("echo \"DQM_LIB_PATH=%s\" >> %s/Makefile ",
                      config.get("DQM_LIB_PATH").c_str(),
                      _output_dir.c_str()).c_str());
  system(Belle2::form("cat Makefile.template >> %s/Makefile",
                      _output_dir.c_str()).c_str());
  system(Belle2::form("make -C %s/ ", _output_dir.c_str()).c_str());
}

std::string MonitorGenerator::createHeader()
{
  const std::string class_name = _name + "Monitor";
  std::string file_path = _output_dir + "/include/" + class_name + ".h";
  std::ofstream fout(file_path.c_str());
  fout << "#ifndef _Belle2_" << class_name << "_h" << std::endl
       << "#define _Belle2_" << class_name << "_h" << std::endl
       << std::endl
       << "#include <dqm/AbstractMonitor.h>" << std::endl
       << std::endl
       << "namespace Belle2 {" << std::endl
       << std::endl
       << "  class " << class_name << " : public AbstractMonitor {" << std::endl
       << std::endl
       << "  public:" << std::endl
       << "    " << class_name << "();" << std::endl
       << "    virtual ~" << class_name << "() throw() {};" << std::endl
       << std::endl
       << "  public:" << std::endl
       << "    virtual void init() throw();" << std::endl
       << "    virtual void update(NSMData* data) throw();" << std::endl
       << std::endl
       << "  };" << std::endl
       << std::endl
       << "}" << std::endl
       << std::endl
       << "#endif" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}

std::string MonitorGenerator::createSource()
{
  const std::string class_name = _name + "Monitor";
  std::string file_path = _output_dir + "/src/" + class_name + ".cc";
  std::ofstream fout(file_path.c_str());
  fout << "#include \"" << class_name << ".h\"" << std::endl
       << std::endl
       << "using namespace Belle2;" << std::endl
       << std::endl
       << class_name << "::" << class_name
       << "()" << std::endl
       << "  : AbstractMonitor(\"" << class_name << "\")" << std::endl
       << "{" << std::endl
       << std::endl
       << "}" << std::endl
       << std::endl
       << "void " << class_name << "::init() throw()" << std::endl
       << "{" << std::endl
       << "  " << std::endl
       << "}" << std::endl
       << std::endl
       << "void " << class_name << "::update(NSMData* data) throw()" << std::endl
       << "{" << std::endl
       << "  " << std::endl
       << "}" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}

std::string MonitorGenerator::createLibSource()
{
  const std::string class_name = _name + "Monitor";
  std::string file_path = _output_dir + "/src/lib" + class_name + ".cc";
  std::ofstream fout(file_path.c_str());
  fout << "#include \"" << class_name << ".h\"" << std::endl
       << std::endl
       << "extern \"C\" void* create" << class_name << "() {" << std::endl
       << "  return new Belle2::" << class_name << "();" << std::endl
       << "}" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}
