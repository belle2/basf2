#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/HLTFramework.h>

using namespace Belle2;

int validationCommands(int argc, char** argv);
void showUsage(void);

int main(int argc, char** argv)
{
  int validation = validationCommands(argc, argv);

  // arguments exceptions
  if (validation < 0)
    return validation;
  else if (validation > 2) {
    B2WARNING("No explicit error but unexpected usage of the software!");
    return validation;
  }
  // no exceptions, then start working
  else {
    HLTFramework* hltFramework = new HLTFramework(validation);
    hltFramework->init(argv[2]);

    delete hltFramework;
  }

  return 0;
}

int validationCommands(int argc, char** argv)
{
  // If there's no arguments
  if (argc < 2) {
    showUsage();
    return -1;
  }

  // mode == manager
  if (!strcmp(argv[1], "manager")) {
    if (!argv[2]) {
      B2ERROR("No HLT information XML file is assigned!");
      showUsage();
      return -1;
    } else {
      return 1;
    }
  }
  // mode == node
  else if (!strcmp(argv[1], "node")) {
    return 2;
  } else {
    B2ERROR("Invalid node type is assigned: Only manager or node are relavant!");
    showUsage();

    return -1;
  }
}

void showUsage()
{
  B2ERROR("Usage: $ hbasf2 [mode] {input}");
  B2ERROR("                mode:  {manager, node}");
  B2ERROR("                input: Node information XML file");
}
