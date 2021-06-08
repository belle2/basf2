#pragma once
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>

namespace Belle2::group_helper {

  template <class _Ty>
  using _Remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;


  template <typename T>
  struct axis_name_t {
    T m_value = T{};

    using my_type = T;
    axis_name_t() = default;
    explicit axis_name_t(T val) : m_value(val) {}

    operator T() const
    {
      return m_value;
    }

  };


#define  AXIS_NAME(name_, type_) \
  struct name_; \
  struct name_ : axis_name_t<type_> { \
    name_(type_ val) : axis_name_t<type_>(val) {}  \
    template <typename... T1>\
    name_(const std::tuple <T1...> & val) :name_( std::get<name_>(val)) {}  \
    name_() = default; \
    template <typename T1> \
    auto operator()(const T1& val){ \
      m_value = std::get<name_>(val); \
      return *this; \
    } \
    template <typename T1>\
    auto operator==(const  T1& rhs ) const -> decltype( std::get<name_>(rhs) == m_value ){\
      return std::get<name_>(rhs) == m_value ;\
    }\
  };




  template <typename T, typename... FUNC_T>
  auto fill_vector(T& klmDigits, FUNC_T&& ... func)
  {
    std::vector<std::tuple<  decltype(func(klmDigits[0]))...  >> hits;
    for (int i = 0; i < klmDigits.getEntries(); ++i) {
      hits.emplace_back(func(klmDigits[i])...);
    }
    return hits;
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
    static auto   __isEequal(const VEC_T& vecA, const VEC_T& vecB) -> decltype(std::enable_if_t< (bool)sizeof...(T_rest), bool> {})
    {
      if (std::get<T1>(vecA) != std::get<T1>(vecB)) {
        return false;
      }
      return __isEequal< VEC_T, T_rest...>(vecA, vecB);
    }


    template <typename VEC_T, typename T1>
    static bool __isEequal(const VEC_T& vecA, const VEC_T& vecB)
    {
      return std::get<T1>(vecA) == std::get<T1>(vecB);
    }

    template <typename VEC_T, typename... FUNC_T>
    static auto apply(const std::vector<VEC_T>& vec, FUNC_T&& ... fun)
    {

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

    template <typename A1 , typename... ARGGS>
    struct __get_element {

      template<class ARRAY_T>
      static constexpr auto has_safe_access(ARRAY_T&& arr) -> decltype(arr.at(0), std::true_type());
      static constexpr auto has_safe_access(...)->std::false_type;

      template <class ARRAY_T>
      static auto& get_safe_if_possible(ARRAY_T& arr, size_t i)
      {
        if constexpr(decltype(has_safe_access(arr))::value) {
          return arr.at(i);
        } else {
          return arr[i];
        }
      }



      template <typename T1, typename T2>
      static auto& get(const T1& e, T2& out_array)
      {
        if constexpr(sizeof...(ARGGS)) {
          return __get_element<ARGGS...>::get(e,
                                              get_safe_if_possible(out_array, std::get<A1>(e))
                                             );
        } else {
          return get_safe_if_possible(out_array, std::get<A1>(e));
        }
      }

    };

    template <typename T1, typename T2, typename T3, typename FUNC_T>
    static auto to_array(const  T2& in_array, T3& out_array, FUNC_T&& fun)
    {
      for (const auto& e : in_array) {
        auto& x = __get_element<T...>::get(e, out_array);
        x = fun(x, std::get<T1>(e));
      }
    }
    template <typename T1, typename T2, typename T3>
    static auto to_array(const  T2& in_array, T3& out_array)
    {
      return to_array<T1>(in_array, out_array, std::plus());
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

  struct plus {
    template<typename T, typename U>
    constexpr auto operator()(T&& t, U&& u) const -> decltype((T&&) t + (U&&) u)
    {
      return (T&&) t + (U&&) u;
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
      return m_data <= (U&&) u;
    }
  };
  struct identity {
    template<typename T>
    constexpr T&& operator()(T&& t) const noexcept
    {
      return (T&&) t;
    }
  };


  template <typename CONTAINER_T, typename INIT_T, typename OP_T = plus, typename PROJECTION_T =  identity>
  auto accumulate(const  CONTAINER_T& container, INIT_T init, OP_T op = plus {} , PROJECTION_T proj = identity{})
  {
    for (const auto& e : container)
      init = op(init, proj(e));
    return init;
  }

  template <typename CONTAINER_T, typename OP_T = group_helper::greater<int>, typename PROJECTION_T =  identity>
  auto count_if(const  CONTAINER_T& container, OP_T op = group_helper::greater<int> {0} , PROJECTION_T proj = identity{})
  {
    int i = 0;
    for (const auto& e : container)
      if (op(proj(e)))
        ++i;
    return i;
  }

  template <typename CONTAINER_T, typename CONDITION_T, typename DEFAULT_T = int, typename PROJECTION_T =  identity>
  auto first_or_default(const  CONTAINER_T& container, const CONDITION_T& con,  DEFAULT_T default__ = 0,
                        PROJECTION_T proj = identity {})
  {
    decltype(proj(container[0])) ret = default__;
    for (const auto& e : container) {
      if (con == e) {
        return proj(e);
      }
    }
    return ret;
  }
}