#include <skim/softwaretrigger/dataobjects/SoftwareTriggerResult.h>

using namespace Belle2;
using namespace SoftwareTrigger;

/*std::ostream &operator<<(std::ostream &os, SoftwareTriggerCutResult const &result) {
  std::string printableText;
  switch (result) {
    case SoftwareTriggerCutResult::c_accept:
      printableText = "accepted";
      break;
    case SoftwareTriggerCutResult::c_noResult:
      printableText = "no result";
      break;
    case SoftwareTriggerCutResult::c_reject:
      printableText = "rejected";
      break;
    default:
      B2FATAL("Invalid result");
      break;
  }

  return os << printableText;
}*/