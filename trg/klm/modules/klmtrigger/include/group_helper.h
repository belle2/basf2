/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef group_helper_h
#define group_helper_h


#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <type_traits>
#include <iostream>






namespace Belle2 {
  namespace group_helper {


    template <auto Start, auto End, auto Inc, class F>
    constexpr void constexpr_for(F&& f)
    {
      if constexpr(Start < End) {
        f(std::integral_constant<decltype(Start), Start>());
        constexpr_for < Start + Inc, End, Inc > (f);
      }
    }

    inline std::string root_type_name(int)
    {
      return "I";
    }

    inline std::string root_type_name(double)
    {
      return "D";
    }
    inline std::string root_type_name(float)
    {
      return "F";
    }
    inline std::string root_type_name(int64_t)
    {
      return "L";
    }
    inline std::string root_type_name(uint64_t)
    {
      return "l";
    }










#define  AXIS_NAME1(name_, type_, name_str) \
  namespace name_##_impl_{\
    struct name_##_; \
    struct name_##_  { \
      type_ name_ = type_{}; \
      constexpr explicit name_##_(type_ val) : name_ (val) {}  \
      template <typename... T1>\
      constexpr  explicit name_##_(const std::tuple <T1...> & val) : name_ ( std::get<name_##_>(val)) {}  \
      constexpr name_##_() = default; \
      template <typename... T1> \
      auto operator()(const std::tuple<T1...>& val) const{ \
        return std::get<name_##_>(val); \
      } \
      constexpr operator type_() const  {\
        return name_ ; \
      } \
      constexpr type_ value() const  {\
        return name_ ; \
      } \
      template <typename... T1>\
      auto operator==(const  std::tuple<T1...>& rhs ) const -> decltype( std::get<name_##_>(rhs) == name_  ){\
        return std::get<name_##_>(rhs) == name_  ;\
      }\
      friend inline  std::string __get__name__(const name_##_&){ return name_str ;}\
      friend inline  std::string __get__name__and_type(const name_##_&){ return std::string(name_str) +"/"+ Belle2::group_helper::root_type_name(type_());}\
      friend inline  std::ostream& operator<<(std::ostream& out, const name_##_ & element ){ out << __get__name__(element) << ":  "<<  element.value() ; return out; }\
    };}\
  using name_ = name_##_impl_::name_##_



#define  AXIS_NAME(name_, type_)  AXIS_NAME1(name_, type_, #name_ )


    namespace group_helper_imple__ {
      template <int N>
      struct print_s {
        template <typename T >
        static void print_name(std::ostream& out, const T& tup)
        {
          if constexpr(N < std::tuple_size<T> ()) {
            out << "| " << __get__name__(std::get<N>(tup)) << " ";
            print_s < N + 1 >::print_name(out, tup);
          } else {
            out <<  "|";
          }
        }
        template <typename T, typename FUNC_T>
        static void print(std::ostream& out, const T& tup, const  FUNC_T& fun)
        {
          if constexpr(N < std::tuple_size<T> ()) {
            out << "| " << fun(std::get<N>(tup)) << " ";
            print_s < N + 1 >::print(out, tup, fun);
          } else {
            out <<  "|";
          }


        }
      };

    }

    template <typename... ARGS>
    std::ostream& operator<<(std::ostream& out, const std::tuple<ARGS...>& tup)
    {
      group_helper_imple__::print_s<0>::print(out, tup, [](const auto & e) { return e;});
      return out;
    }

    template <typename... ARGS>
    std::ostream& operator<<(std::ostream& out, const std::vector<std::tuple<ARGS...>>& vec)
    {
      decltype(vec[0]) header{};
      group_helper_imple__::print_s<0>::print_name(out, header);
      out << "\n";
      for (const auto& ele : vec) {
        group_helper_imple__::print_s<0>::print(out, ele, [](const auto & e) { return e.value() ;});
        out << "\n";
      }

      return out;
    }




    template<typename T, typename... Ts>
    constexpr bool contains()
    {
      return std::disjunction_v<std::is_same<T, Ts>...>;
    }

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T, typename... Us>
    struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};




    template <class _Ty>
    using _Remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;

    template <typename... FUNC_T>
    auto fill_vector(size_t entries, FUNC_T&& ... func)
    {
      std::vector<std::tuple<  decltype(func(size_t(0)))...  >> ret ;
      ret.reserve(entries);
      for (size_t i = 0; i < entries; ++i) {
        ret.emplace_back(func(i)...);
      }
      return ret;
    }


    template <typename T1, typename T2>
    class __range__impl {

    public:

      __range__impl(T1&& b, T2&& e) : m_begin(b), m_end(e) {}
      __range__impl(const T1& b, const T2& e) : m_begin(b), m_end(e) {}
      auto begin() const
      {
        return m_begin;
      }
      auto back() const
      {
        return *(m_end - 1);
      }
      auto front() const
      {
        return *(m_begin);
      }
      auto end() const
      {
        return m_end;
      }
      auto operator[](size_t i) const
      {
        return *(m_begin + i);
      }
      size_t size() const
      {
        return  m_end - m_begin;
      }
      T1 m_begin;
      T2 m_end;

    };


    template <typename T1, typename T2>
    auto __range__(T1&& b, T2&& e)
    {
      return __range__impl< _Remove_cvref_t<T1>, _Remove_cvref_t<T2> >(std::forward<T1>(b), std::forward<T2>(e));
    }

    template <typename... T>
    struct group {

      template <typename VEC_T, typename T1, typename... T_rest>
      static auto   __isEequal(const VEC_T& vecA, const VEC_T& vecB)
      {
        if constexpr(sizeof...(T_rest) > 0) {
          if (std::get<T1>(vecA) != std::get<T1>(vecB)) {
            return false;
          }
          return __isEequal< VEC_T, T_rest...>(vecA, vecB);
        } else {
          return std::get<T1>(vecA) == std::get<T1>(vecB);
        }
      }

      template <typename VEC_T, typename T1, typename... T_rest>
      static auto   __isLessthen(const VEC_T& vecA, const VEC_T& vecB)
      {
        if constexpr(sizeof...(T_rest) > 0) {
          if (std::get<T1>(vecA) < std::get<T1>(vecB)) {
            return true;
          } else if (std::get<T1>(vecA) > std::get<T1>(vecB)) {
            return false;
          }
          return __isLessthen< VEC_T, T_rest...>(vecA, vecB);
        } else {
          return std::get<T1>(vecA) < std::get<T1>(vecB);
        }
      }



      template <typename VEC_T, typename... FUNC_T>
      static auto apply(const std::vector<VEC_T>& vec, FUNC_T&& ... fun)
      {

        auto is_sorted_ = std::is_sorted(std::begin(vec), std::end(vec), [](const auto & lhs, const auto & rhs) {
          return group<T...>::__isLessthen<VEC_T, T...>(lhs, rhs);
        });

        if (!is_sorted_) {
          throw std::runtime_error("group::apply expects the input vector to be sorted according to the group parameters\nthe input vector was not sorted with respect to the group axis");
        }


        std::vector< std::tuple<T..., decltype(fun(__range__(std::begin(vec), std::end(vec))))...  >> ret;
        if (vec.empty()) {
          return ret;
        }
        auto tail = std::begin(vec);

        for (auto head = std::begin(vec); head != std::end(vec); ++head) {
          if (!group<T...>::__isEequal<VEC_T, T...>(*head, *tail)) {

            ret.emplace_back(std::get<T>(*tail)..., fun(__range__(tail, head))...);
            tail = head;
          }
        }


        ret.emplace_back(std::get<T>(*tail)..., fun(__range__(tail, std::end(vec)))...);
        return ret;
      }

      template <typename VEC_T, typename FUNC_T>
      static auto apply1(const std::vector<VEC_T>& vec, FUNC_T&& fun)
      {

        auto is_sorted_ = std::is_sorted(std::begin(vec), std::end(vec), [](const auto & lhs, const auto & rhs) {
          return group<T...>::__isLessthen<VEC_T, T...>(lhs, rhs);
        });

        if (!is_sorted_) {
          throw std::runtime_error("group::apply1 expects the input vector to be sorted according to the group parameters\nthe input vector was not sorted with respect to the group axis");
        }

        std::vector <
        decltype(
          tuple_cat(
            std::make_tuple(std::get<T>(*vec.begin())...),
            fun(__range__(std::begin(vec), std::end(vec)))
          )
        )
        > ret;
        if (vec.empty()) {
          return ret;
        }
        auto tail = std::begin(vec);

        for (auto head = std::begin(vec); head != std::end(vec); ++head) {
          if (!group<T...>::__isEequal<VEC_T, T...>(*head, *tail)) {


            auto t = tuple_cat(
                       std::make_tuple(std::get<T>(*tail)...),
                       fun(__range__(tail, head))
                     );

            ret.push_back(t);
            tail = head;
          }
        }
        ret.push_back(tuple_cat(std::make_tuple(std::get<T>(*tail)...), fun(__range__(tail, std::end(vec)))));
        return ret;
      }
    };

    template <typename CONTAINER_T, typename FUNC_T>
    void erase_remove_if(CONTAINER_T& container, FUNC_T&& fun)
    {
      container.erase(
        std::remove_if(container.begin(), container.end(),
                       std::forward<FUNC_T>(fun)),
        container.end()
      );
    }

    template <typename CONTAINER_T>
    void sort(CONTAINER_T& container)
    {
      std::sort(container.begin(), container.end());
    }

    template <typename CONTAINER_T>
    void drop_duplicates(CONTAINER_T& container)
    {
      container.erase(std::unique(container.begin(), container.end()), container.end());
    }

    template <typename CONTAINER_T, typename FUNCTION_T>
    bool contains_if(const CONTAINER_T& container, FUNCTION_T&& fun)
    {
      return std::find_if(container.begin(), container.end(), fun) != container.end();
    }

    template <typename CONTAINER_T, typename T1>
    bool contains(const CONTAINER_T& container, T1&& value)
    {
      return std::find(container.begin(), container.end(), value) != container.end();
    }

    struct plus {
      template<typename T, typename U>
      constexpr auto operator()(T&& t, U&& u) const -> decltype((T&&)t + (U&&)u)
      {
        return (T&&)t + (U&&)u;
      }
    };

    template<typename T1 = int>
    struct greater {
      explicit greater(T1&& data) : m_data(data) {}
      explicit greater(const T1& data) : m_data(data) {}
      T1 m_data;


      constexpr auto operator()(const T1& u) const
      {
        return m_data < u;
      }
    };

    template<typename T1>
    struct greater_equal {
      explicit greater_equal(T1 data) : m_data(data) {}

      T1 m_data;
      template<typename U>
      constexpr auto operator()(U&& u) const
      {
        return m_data <= (U&&)u;
      }
    };
    struct identity {
      template<typename T>
      constexpr T&& operator()(T&& t) const noexcept
      {
        return (T&&)t;
      }
    };


    template <typename CONTAINER_T, typename INIT_T, typename OP_T = plus, typename PROJECTION_T = identity>
    auto accumulate(const  CONTAINER_T& container, INIT_T init, OP_T op = plus {}, PROJECTION_T proj = identity{})
    {
      for (const auto& e : container)
        init = op(init, proj(e));
      return init;
    }

    template <typename CONTAINER_T, typename OP_T = group_helper::greater<int>, typename PROJECTION_T = identity>
    auto count_if(const  CONTAINER_T& container, OP_T op = group_helper::greater<int> { 0 }, PROJECTION_T proj = identity{})
    {
      int i = 0;
      for (const auto& e : container)
        if (op(proj(e)))
          ++i;
      return i;
    }


    template <typename CONTAINER_T, typename CONDITION_T>
    auto get_first(const  CONTAINER_T& container, const CONDITION_T& con)
    {

      for (const auto& e : container) {
        if (con == e)
          return e;

      }
      static decltype(container[0])  ret{};
      return ret;
    }






    namespace comparators {





      struct on_common_args_t {


        template <typename T1, typename T2>
        constexpr static bool __comp__(T1&& t1, T2&& t2)
        {
          bool ret = true;
          constexpr_for<0, std::tuple_size<_Remove_cvref_t<T1> >::value, 1>(
          [&](const  auto i) {
            using N_th_T = _Remove_cvref_t < decltype(std::get<i>(t1))>;
            if constexpr(has_type<N_th_T, _Remove_cvref_t<T2>>::value) {
              ret &= (std::get<N_th_T>(t1) == std::get<N_th_T>(t2));
            }
          }
          );
          return ret;

        }


        template <typename T1, typename T2>
        constexpr bool operator()(T1&& t1, T2&& t2) const
        {
          return __comp__(std::forward<T1>(t1), std::forward<T2>(t2));
        }
      };
      constexpr on_common_args_t on_common_args;
    }


    template <typename T0, typename T1, typename T2, typename Comparision_T, typename projecttion_t >
    void vec_join_r(T0& ret, const T1& t1, const T2& t2, Comparision_T comp, projecttion_t project)
    {
      ret.clear();
      for (const auto& e1 : t1) {
        for (const auto& e2 : t2) {
          if (comp(e1, e2)) {
            ret.push_back(project(e1, e2));
          }
        }
      }

    }

    template <typename T1, typename T2, typename Comparision_T, typename projecttion_t >
    auto vec_join(const T1& t1, const T2& t2, Comparision_T comp, projecttion_t project)
    {
      std::vector<  decltype(project(t1[0], t2[0]))  > ret;
      vec_join_r(ret, t1, t2, comp, project);
      return ret;
    }

  }
}

#endif