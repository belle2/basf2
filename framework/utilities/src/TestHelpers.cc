#include <framework/utilities/TestHelpers.h>

#include <boost/filesystem.hpp>

using namespace Belle2::TestHelpers;
using namespace boost::filesystem;

TempDirCreator::TempDirCreator()
{
  m_oldpwd = current_path().string();
  path tmpdir = temp_directory_path() / unique_path();
  create_directories(tmpdir);
  current_path(tmpdir);
  m_tmpdir = tmpdir.string();
}

TempDirCreator::~TempDirCreator()
{
  current_path(m_oldpwd);
  remove_all(m_tmpdir);
}
