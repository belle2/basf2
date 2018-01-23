#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Path.h>
#include <boost/optional.hpp>

template class Belle2::ModuleParam<bool>;
template class Belle2::ModuleParam<char>;
template class Belle2::ModuleParam<unsigned char>;
template class Belle2::ModuleParam<short>;
template class Belle2::ModuleParam<unsigned short>;
template class Belle2::ModuleParam<int>;
template class Belle2::ModuleParam<unsigned int>;
template class Belle2::ModuleParam<long>;
template class Belle2::ModuleParam<unsigned long>;
template class Belle2::ModuleParam<long long>;
template class Belle2::ModuleParam<unsigned long long>;
template class Belle2::ModuleParam<float>;
template class Belle2::ModuleParam<double>;
template class Belle2::ModuleParam<std::string>;

template class Belle2::ModuleParam<std::vector<bool>>;
template class Belle2::ModuleParam<std::vector<char>>;
template class Belle2::ModuleParam<std::vector<unsigned char>>;
template class Belle2::ModuleParam<std::vector<short>>;
template class Belle2::ModuleParam<std::vector<unsigned short>>;
template class Belle2::ModuleParam<std::vector<int>>;
template class Belle2::ModuleParam<std::vector<unsigned int>>;
template class Belle2::ModuleParam<std::vector<long>>;
template class Belle2::ModuleParam<std::vector<unsigned long>>;
template class Belle2::ModuleParam<std::vector<long long>>;
template class Belle2::ModuleParam<std::vector<unsigned long long>>;
template class Belle2::ModuleParam<std::vector<float>>;
template class Belle2::ModuleParam<std::vector<double>>;
template class Belle2::ModuleParam<std::vector<std::string>>;

template class Belle2::ModuleParam<boost::optional<std::string>>;

template class Belle2::ModuleParam<std::shared_ptr<Belle2::Path>>;
