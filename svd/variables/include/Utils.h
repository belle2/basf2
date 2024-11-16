#pragma once

#include <svd/dataobjects/SVDCluster.h>
#include <svd/variables/ClusterVariables.h>
#include <svd/variables/Variable.h>

// namespace Belle2::SVD::Variables
// {

//   double clusterCharge2(const SVDCluster*);


//   // TODO: This is just an exemplary implementation. For proper function name and return type resolution
//   // we need metaprogramming or something like that. hmmm...?
//   FunctionPtr getFunctionPtr(const std::string& name)
//   {
//     if (name == "clusterCharge")
//     {
//       return clusterCharge;
//     }
//     else
//     {
//       return nullptr;
//     }
//   }

//   VariableDataType getVariableDataType(const std::string& name)
//   {
//     if (name == "clusterCharge")
//     {
//       return VariableDataType::c_double;
//     }
//     else
//     {
//       return VariableDataType::c_int;
//     }
//   }
// }