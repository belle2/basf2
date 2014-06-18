/**
 * Thomas Keck 2014
 */
#include "analysis/TMVAMockPluginInspector/MockPluginInspector.h"

Belle2::MockPluginInspector& Belle2::MockPluginInspector::GetInstance() { static Belle2::MockPluginInspector inspector; return inspector; }

