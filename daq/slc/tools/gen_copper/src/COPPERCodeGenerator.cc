#include "COPPERCodeGenerator.hh"

#include <node/COPPERNode.hh>
#include <node/HSLB.hh>
#include <node/FEEModule.hh>

#include <util/StringUtil.hh>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <algorithm>
#include <string>

using namespace B2DAQ;

void COPPERCodeGenerator::create()
{
  std::cout << "making directory: " << _output_dir << std::endl;
  system(std::string(("mkdir -p ") + _output_dir).c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/include").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/src").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/tmp").c_str());
  system(std::string(("mkdir -p ") + _output_dir + "/lib").c_str());
  std::cout << "copying makefiles: " << _output_dir << std::endl;
  system(B2DAQ::form("cp Makefile.template %s/Makefile",
                     _output_dir.c_str()).c_str());
  for (std::map<std::string, COPPERNode*>::iterator it = _copper_m.begin();
       it != _copper_m.end(); it++) {
    setCOPPER(it->second);
    setModuleClass(it->first);
    std::cout << "creating header file for struct : "
              << createDataHeader() << std::endl;
    std::cout << "creating header file for copper node class : "
              << createDataHandlerHeader() << std::endl;
    std::cout << "creating source file for copper node class : "
              << createDataHandlerSource() << std::endl;
    std::cout << "creating source file for dynamic loader of copper node : "
              << createDynamicLoadSource() << std::endl;
    system(B2DAQ::form("make -C %s/ USER_CLASS=%s",
                       _output_dir.c_str(),
                       _module_class.c_str()).c_str());
  }
}

std::string COPPERCodeGenerator::createDataHeader()
{
  const std::string module_class = B2DAQ::tolower(_module_class);
  const std::string struct_name = module_class + "_data";
  std::string file_path = _output_dir + "/include/" + struct_name + ".h";
  std::ofstream fout(file_path.c_str());
  int revision = (_copper->getVersion() > 0) ? _copper->getVersion() : 1;
  fout << "#ifndef _B2DAQ_" << struct_name << "_h" << std::endl
       << "#define _B2DAQ_" << struct_name << "_h" << std::endl
       << std::endl
       << "const int " << struct_name
       << "_revision = " << revision << ";" << std::endl
       << std::endl
       << "struct " << struct_name << " {" << std::endl
       << "  uint32   id;" << std::endl
       << "  char  sender_script[64];" << std::endl
       << "  int32 sender_port;" << std::endl
       << "  int32 sender_event_size;" << std::endl
       << "  byte8 hslb_used[4];" << std::endl
       << "  char  hslb_firmware[128]; // 32 * 4" << std::endl;
  for (int slot = 0; slot < 4; slot++) {
    HSLB* hslb = _copper->getHSLB(slot);
    if (hslb == NULL || hslb->getFEEModule() == NULL) continue;
    fout << "  // slot " << (char)('a' + slot)
         << " : module_type = " << hslb->getFEEModule()->getType() << std::endl;
    const std::vector<FEEModule::Register>& reg_v(hslb->getFEEModule()->getRegisters());
    for (size_t i = 0; i < reg_v.size(); i++) {
      const FEEModule::Register& reg(reg_v[i]);
      if (reg.length() > 1) {
        fout << "  int32 " << reg.getName() << "_" << (char)('a' + slot)
             << "[" << reg.length() << "];" << std::endl;
      } else {
        fout << "  int32 " << reg.getName() << "_" << (char)('a' + slot)
             << ";" << std::endl;
      }
    }
  }
  fout << "};" << std::endl
       << std::endl
       << "#endif" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}

std::string COPPERCodeGenerator::createDataHandlerHeader()
{
  const std::string class_name = _module_class + "Data";
  std::string file_path = _output_dir + "/include/" + class_name + ".hh";
  std::ofstream fout(file_path.c_str());
  fout << "#ifndef _B2DAQ_" << class_name << "_hh" << std::endl
       << "#define _B2DAQ_" << class_name << "_hh" << std::endl
       << std::endl
       << "#include <node/COPPERNode.hh>" << std::endl
       << std::endl
       << "#include <nsm/NSMData.hh>" << std::endl
       << std::endl
       << "namespace B2DAQ {" << std::endl
       << std::endl
       << "  class " << class_name << " : public NSMData {" << std::endl
       << std::endl
       << "  public:" << std::endl
       << "    " << class_name << "(COPPERNode* node = NULL, const std::string& name=\"\") throw();" << std::endl
       << "    virtual ~" << class_name << "() throw() {};" << std::endl
       << std::endl
       << "  public:" << std::endl
       << "    virtual void read(NSMNode* node) throw(NSMHandlerException);" << std::endl
       << "    virtual void write(NSMNode* node) throw(NSMHandlerException);" << std::endl
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

std::string COPPERCodeGenerator::createDataHandlerSource()
{
  const std::string module_class = B2DAQ::tolower(_module_class);
  const std::string struct_name = module_class + "_data";
  const std::string class_name = _module_class + "Data";
  std::string file_path = _output_dir + "/src/" + class_name + ".cc";
  std::ofstream fout(file_path.c_str());
  fout << "#include \"" << class_name << ".hh\"" << std::endl
       << std::endl
       << "#include <node/HSLB.hh>" << std::endl
       << "#include <node/FEEModule.hh>" << std::endl
       << "#include <node/DataSender.hh>" << std::endl
       << std::endl
       << "#include <util/StringUtil.hh>" << std::endl
       << std::endl
       << "extern \"C\" {" << std::endl
       << "#include <nsm2.h>" << std::endl
       << "#include \"" << struct_name << ".h\"" << std::endl
       << "}" << std::endl
       << std::endl
       << "#include <cstring>" << std::endl
       << std::endl
       << "using namespace B2DAQ;" << std::endl
       << std::endl
       << class_name << "::" << class_name
       << "(COPPERNode* node, const std::string& data_name) throw()" << std::endl
       << "  : NSMData(data_name, \"" << struct_name << "\", " << struct_name << "_revision) {" << std::endl
       << "  int slot = 0;" << std::endl
       << "  HSLB* hslb = NULL;" << std::endl
       << "  FEEModule* module = NULL;" << std::endl
       << "  FEEModule::Register reg;" << std::endl;
  for (int slot = 0; slot < 4; slot++) {
    HSLB* hslb = _copper->getHSLB(slot);
    if (hslb == NULL || hslb->getFEEModule() == NULL) continue;
    fout << "  " << std::endl
         << "  // -- slot " << (char)('a' + slot) << std::endl
         << "  slot = " << slot << ";" << std::endl
         << "  if ( node->getHSLB(slot) == NULL ) {" << std::endl
         << "    hslb = new HSLB();" << std::endl
         << "    node->setHSLB(slot, hslb);" << std::endl
         << "  } else { " << std::endl
         << "    hslb = node->getHSLB(slot); " << std::endl
         << "  }" << std::endl
         << "  if ( hslb->getFEEModule() == NULL ) {" << std::endl
         << "    module = new FEEModule();" << std::endl
         << "    hslb->setFEEModule(module);" << std::endl
         << "  } else { " << std::endl
         << "    module = hslb->getFEEModule(); " << std::endl
         << "  }" << std::endl ;
    const std::vector<FEEModule::Register>& reg_v(hslb->getFEEModule()->getRegisters());
    for (size_t i = 0; i < reg_v.size(); i++) {
      const FEEModule::Register& reg(reg_v[i]);
      fout << "  reg = FEEModule::Register();" << std::endl
           << "  reg.set(\"" << reg.getName() << "\", " << B2DAQ::form("0x%x", reg.getAddress())
           << ", " << reg.length() << ", " << reg.getSize() << ");" << std::endl
           << "  module->addRegister(reg);" << std::endl;

    }
  }
  fout << std::endl
       << "}" << std::endl
       << std::endl
       << "void " << class_name << "::read(NSMNode* node) throw(NSMHandlerException) {" << std::endl
       << "  COPPERNode* copper = (COPPERNode*)node;" << std::endl
       << B2DAQ::form("  %s* data = (%s*)get()"";", struct_name.c_str(), struct_name.c_str()) << std::endl
       << "  copper->setID(data->id);" << std::endl
       << "  DataSender* sender = copper->getSender();" << std::endl
       << "  if ( sender != NULL ) {" << std::endl
       << "    sender->setScript(data->sender_script);" << std::endl
       << "    sender->setPort(data->sender_port);" << std::endl
       << "    sender->setEventSize(data->sender_event_size);" << std::endl
       << "  }" << std::endl
       << "  HSLB* hslb = NULL;" << std::endl
       << "  FEEModule* module = NULL;" << std::endl
       << "  int slot;" << std::endl;
  for (int slot = 0; slot < 4; slot++) {
    HSLB* hslb = _copper->getHSLB(slot);
    if (hslb == NULL || hslb->getFEEModule() == NULL) continue;
    fout << "  // -- slot " << (char)('a' + slot) << std::endl
         << "  slot = " << slot << ";" << std::endl
         << "  hslb = copper->getHSLB(slot);" << std::endl
         << "  hslb->setUsed(data->hslb_used[slot]);" << std::endl
         << "  if ( hslb != NULL && hslb->getFEEModule() != NULL ) {" << std::endl
         << "    if ( hslb->isUsed() ) {" << std::endl
         << "      hslb->setFirmware((const char*)(data->hslb_firmware + 32 * slot));" << std::endl
         << "      module = hslb->getFEEModule();" << std::endl;
    const std::vector<FEEModule::Register>& reg_v(hslb->getFEEModule()->getRegisters());
    for (size_t i = 0; i < reg_v.size(); i++) {
      const FEEModule::Register& reg(reg_v[i]);
      if (reg.length() > 1) {
        fout << B2DAQ::form("      for ( int ch = 0; ch < %d; ch++ ) {", reg.length()) << std::endl
             << B2DAQ::form("        module->getRegister(%d).setValue(ch, data->%s_%c[ch]);",
                            i, reg.getName().c_str(), 'a' + slot) << std::endl
             << "      }" << std::endl;
      } else {
        fout << B2DAQ::form("      module->getRegister(%d).setValue(0, data->%s_%c);",
                            i, reg.getName().c_str(), 'a' + slot) << std::endl;
      }
    }
    fout << "    }" << std::endl
         << "  }" << std::endl;
  }
  fout << "}" << std::endl
       << std::endl
       << "void " << class_name << "::write(NSMNode* node) throw(NSMHandlerException) {" << std::endl
       << "  COPPERNode* copper = (COPPERNode*)node;" << std::endl
       << B2DAQ::form("  %s* data = (%s*)get()"";", struct_name.c_str(), struct_name.c_str()) << std::endl
       << "  data->id = copper->getID();" << std::endl
       << "  DataSender* sender = copper->getSender();" << std::endl
       << "  if ( sender != NULL ) {" << std::endl
       << "    strncpy(data->sender_script, sender->getScript().c_str(), 64);" << std::endl
       << "    data->sender_port = sender->getPort();" << std::endl
       << "    data->sender_event_size = sender->getEventSize();" << std::endl
       << "  }" << std::endl
       << "  HSLB* hslb = NULL;" << std::endl
       << "  FEEModule* module = NULL;" << std::endl
       << "  int slot;" << std::endl;
  for (int slot = 0; slot < 4; slot++) {
    HSLB* hslb = _copper->getHSLB(slot);
    if (hslb == NULL || hslb->getFEEModule() == NULL) continue;
    fout << "  // -- slot " << (char)('a' + slot) << std::endl
         << "  slot = " << slot << ";" << std::endl
         << "  hslb = copper->getHSLB(slot);" << std::endl
         << "  data->hslb_used[slot] = (byte8)hslb->isUsed();" << std::endl
         << "  if ( hslb != NULL && hslb->getFEEModule() != NULL ) {" << std::endl
         << "    if ( hslb->isUsed() ) {" << std::endl
         << "      strncpy((char*)(data->hslb_firmware + 32 * slot),"
         << " hslb->getFirmware().c_str(), 32);" << std::endl
         << "      module = hslb->getFEEModule();" << std::endl;
    const std::vector<FEEModule::Register>& reg_v(hslb->getFEEModule()->getRegisters());
    for (size_t i = 0; i < reg_v.size(); i++) {
      const FEEModule::Register& reg(reg_v[i]);
      if (reg.length() > 1) {
        fout << B2DAQ::form("      for ( int ch = 0; ch < %d; ch++ ) {", reg.length()) << std::endl
             << B2DAQ::form("        data->%s_%c[ch] = module->getRegister(%d).getValue(ch);",
                            reg.getName().c_str(), 'a' + slot, i) << std::endl
             << "      }" << std::endl;
      } else {
        fout << B2DAQ::form("      data->%s_%c = module->getRegister(%d).getValue(0);",
                            reg.getName().c_str(), 'a' + slot, i) << std::endl;
      }
    }
    fout << "    }" << std::endl
         << "  }" << std::endl;
  }
  fout << "}" << std::endl;
  fout << std::endl;
  fout.close();
  return file_path;
}

std::string COPPERCodeGenerator::createDynamicLoadSource()
{
  const std::string struct_name = _module_class + "_data";
  const std::string class_name = _module_class + "Data";
  std::string file_path = _output_dir + "/src/lib" + class_name + ".cc";
  std::ofstream fout(file_path.c_str());
  fout << "#include \"" << class_name << ".hh\"" << std::endl
       << std::endl
       << "#include <node/COPPERNode.hh>" << std::endl
       << std::endl
       << "extern \"C\" void* create" << class_name << "(void* node, const char* name) {" << std::endl
       << "  return new B2DAQ::" << class_name << "((B2DAQ::COPPERNode*)node, name);" << std::endl
       << "}" << std::endl
       << std::endl;
  fout.close();
  return file_path;
}
