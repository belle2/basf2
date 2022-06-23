#pragma  once

#include "TFile.h"
#include "TTree.h"



namespace Belle2::group_helper {
  template <typename T>
  void add_branch_element(TTree* tree, const std::string& prefix,  T& element)
  {
    auto branchName = prefix + __get__name__(element);

    tree->Branch(branchName.c_str(), &element, __get__name__and_type(element).c_str());

  }


  template <int N, typename T>
  auto add_branch(TTree* tree, const std::string& prefix,  T& element)
  {
    add_branch_element(tree, prefix, std::get<N>(element));
    //auto branchName =  __get__name__(std::get<N>(element));

    //tree->Branch(branchName.c_str(), &std::get<N>(element), __get__name__and_type(std::get<N>(element)).c_str());
    if constexpr(N > 0) {
      add_branch < N - 1 > (tree, prefix, element);
    }
  }


  template <int N, typename... T>
  void add_branch(TTree* tree, const std::string& prefix, std::pair<T...>& element)
  {

    add_branch < std::tuple_size_v<decltype(element.first) > -1 > (tree, prefix + "rhs_", element.first);
    add_branch < std::tuple_size_v<decltype(element.second)> -1 > (tree, prefix + "lhs_", element.second);
    //auto branchName = prefix + __get__name__(element);

    //tree->Branch(branchName.c_str(), &element, __get__name__and_type(element).c_str());


  }

  template< class T >
  struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
  };

  template <typename T>
  void save(const std::string& FileName, const T& vec, const std::string& BranchName, const std::string& FileNode = "RECREATE")
  {
    TFile* f = new TFile(FileName.c_str(), FileNode.c_str());

    TTree* tree = new TTree(BranchName.c_str(), BranchName.c_str());
    typename remove_cvref<decltype(*vec.begin())>::type element{};

    add_branch < std::tuple_size_v< decltype(element)> -1 > (tree, "", element);
    for (const auto& e : vec) {
      element = e;
      tree->Fill();
    }

    tree->Write();
    f->Write();
    delete f;
  }


  template <int N, typename T>
  auto SetBranchAddress(TTree* tree, T& element)
  {

    auto i = tree->SetBranchAddress(__get__name__(std::get<N>(element)).c_str(), &std::get<N>(element));
    if (i != TTree::kMatch) {

      std::string error_msg = std::string("Unable to load Branch: ") + __get__name__(std::get<N>(element));
      if (i == TTree::kMissingBranch) {
        error_msg += "\nError = kMissingBranch";
      } else if (i == TTree::kInternalError) {
        error_msg += "\nError = kInternalError";
      } else if (i == TTree::kMissingCompiledCollectionProxy) {
        error_msg += "\nError = kMissingCompiledCollectionProxy";
      } else if (i == TTree::kMismatch) {
        error_msg += "\nError = kMismatch";
      } else if (i == TTree::kClassMismatch) {
        error_msg += "\nError = kClassMismatch";
      } else if (i == TTree::kMatchConversion) {
        error_msg += "\nError = kMatchConversion";
      } else if (i == TTree::kMatchConversionCollection) {
        error_msg += "\nError = kMatchConversionCollection";
      } else if (i == TTree::kMakeClass) {
        error_msg += "\nError = kMakeClass";
      } else if (i == TTree::kVoidPtr) {
        error_msg += "\nError = kVoidPtr";
      } else if (i == TTree::kNoCheck) {
        error_msg += "\nError = kNoCheck";
      } else {
        error_msg += "\nError = unknown";
      }
      throw std::runtime_error(error_msg);
    }


    if constexpr(N > 0) {
      SetBranchAddress < N - 1 > (tree, element);
    }
  }


  template <typename tuple_type>
  auto load_ttree(const std::string& fileName, const std::string& treeName)
  {
    auto file = TFile(fileName.c_str(), "read");
    auto tree = (TTree*)file.Get(treeName.c_str());


    tuple_type buffer;
    SetBranchAddress < std::tuple_size<decltype(buffer)> {} -1 > (tree, buffer);
    std::vector< tuple_type > ret;
    for (int i = 0; i < tree->GetEntries(); ++i) {
      tree->GetEntry(i);
      ret.push_back(buffer);
    }

    return ret;

  }

}