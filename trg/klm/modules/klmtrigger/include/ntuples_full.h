/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <type_traits>
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

namespace nt {

  template <auto Start, auto End, auto Inc, class F>
  constexpr void constexpr_for(F&& f)
  {
    if constexpr(Start < End) {
      f(std::integral_constant<decltype(Start), Start>());
      constexpr_for < Start + Inc, End, Inc > (f);
    }
  }

}




namespace nt {
  template <typename T>
  class span {
  public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    class iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;
      using difference_type = std::ptrdiff_t;
      using pointer = T*;
      using reference = T&;

      iterator(std::byte* ptr, size_type stride)
        : m_ptr(ptr), m_stride(stride) {}

      reference operator*() const { return *reinterpret_cast<pointer>(m_ptr); }
      pointer operator->() const { return reinterpret_cast<pointer>(m_ptr); }

      iterator& operator++()
      {
        m_ptr += m_stride;
        return *this;
      }

      iterator operator++(int)
      {
        iterator temp = *this;
        m_ptr += m_stride;
        return temp;
      }

      friend bool operator==(const iterator& a, const iterator& b)
      {
        return a.m_ptr == b.m_ptr;
      }

      friend bool operator!=(const iterator& a, const iterator& b)
      {
        return a.m_ptr != b.m_ptr;
      }

    private:
      std::byte* m_ptr;
      size_type m_stride;
    };

    span(std::byte* ptr, size_type count, size_type stride = sizeof(T))
      : m_ptr(ptr), m_count(count), m_stride(stride) {}

    reference operator[](size_type idx) const
    {
      return *reinterpret_cast<pointer>(m_ptr + idx * m_stride);
    }

    pointer data() const noexcept
    {
      return reinterpret_cast<pointer>(m_ptr);
    }

    size_type size() const noexcept
    {
      return m_count;
    }

    size_type stride() const noexcept
    {
      return m_stride;
    }

    iterator begin() const noexcept
    {
      return iterator(m_ptr, m_stride);
    }

    iterator end() const noexcept
    {
      return iterator(m_ptr + m_count * m_stride, m_stride);
    }

  private:
    std::byte* m_ptr;
    size_type m_count;
    size_type m_stride;
  };

}

#define nt_span(container, member) nt::span<decltype(container[0].member.v)>(                    \
    container.size() ? (std::byte *)&decltype(container[0].member)::get(container[0]) : nullptr, \
    container.size(), sizeof(decltype(container[0])))

#define nt_span_t(container, member) nt::span<decltype(container[0].member)>(                    \
    container.size() ? (std::byte *)&decltype(container[0].member)::get(container[0]) : nullptr, \
    container.size(), sizeof(decltype(container[0])))

namespace nt {

  template <class _Ty>
  using _Remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;

  template <int N, typename... Ts>
  using NthTypeOf =
    typename std::tuple_element<N, std::tuple<Ts...>>::type;

  template <typename T1>
  struct type_container {
    using type = T1;
  };

  template <typename T1, typename T2>
  struct ax_type : T2 {
    T1 v = {};
    constexpr ax_type() {}
    constexpr ax_type(T1 t1) : v(std::move(t1)) {}
    constexpr ax_type operator=(T1&& t1) const { return ax_type(std::move(t1)); }
    constexpr ax_type operator=(const T1& t1) const { return ax_type(t1); }

    template <typename T_IN>
    constexpr ax_type operator()(T_IN&& t1) const { return ax_type(std::move(static_cast<T1>(t1))); }

    template <typename T_IN>
    constexpr ax_type operator()(const T_IN& t1) const { return ax_type(static_cast<T1>(t1)); }
    constexpr ax_type operator=(T1&& t1)
    {
      v = std::move(t1);
      return *this;
    }
    constexpr ax_type& operator=(const T1& t1)
    {
      v = t1;
      return *this;
    }

    template <typename T>
    constexpr ax_type& operator=(const ax_type<T1, T>& t1)
    {
      v = t1.v;
      return *this;
    }

    using struct_maker = T2;
    using data_t = T1;

    operator T1()
    {
      return v;
    }

    operator const T1() const
    {
      return v;
    }

    template <typename T_RHS>
    friend constexpr bool operator<(const ax_type& lhs, const ax_type<T1, T_RHS>& rhs)
    {
      return lhs.v < rhs.v;
    }

    template <typename T_RHS>
    friend constexpr bool operator<=(const ax_type& lhs, const ax_type<T1, T_RHS>& rhs)
    {
      return lhs.v <= rhs.v;
    }

    template <typename T_RHS>
    friend constexpr bool operator==(const ax_type& lhs, const ax_type<T1, T_RHS>& rhs)
    {
      return lhs.v == rhs.v;
    }

    template <typename T_RHS>
    friend constexpr bool operator!=(const ax_type& lhs, const ax_type<T1, T_RHS>& rhs)
    {
      return lhs.v != rhs.v;
    }

    template <typename T_RHS>
    friend constexpr bool operator>=(const ax_type& lhs, const ax_type<T1, T_RHS>& rhs)
    {
      return lhs.v >= rhs.v;
    }

    template < typename T_RHS>
    friend constexpr bool operator>(const ax_type& lhs, const ax_type<T1, T_RHS>& rhs)
    {
      return lhs.v > rhs.v;
    }

    friend constexpr bool operator<(const ax_type& lhs, const T1& rhs)
    {
      return lhs.v < rhs;
    }
    friend constexpr bool operator<(const T1 lhs, const ax_type& rhs)
    {
      return lhs < rhs.v;
    }

    friend constexpr bool operator<=(const ax_type& lhs, const T1& rhs)
    {
      return lhs.v <= rhs;
    }

    friend constexpr bool operator<=(const T1& lhs, const  ax_type& rhs)
    {
      return lhs <= rhs.v;
    }

    friend constexpr bool operator==(const ax_type& lhs, const T1& rhs)
    {
      return lhs.v == rhs;
    }

    friend constexpr bool operator==(const T1& lhs, const  ax_type& rhs)
    {
      return lhs == rhs.v;
    }

    friend constexpr bool operator!=(const ax_type& lhs, const T1& rhs)
    {
      return lhs.v != rhs;
    }

    friend constexpr bool operator!=(const T1& lhs, const  ax_type& rhs)
    {
      return lhs != rhs.v;
    }

    friend constexpr bool operator>=(const ax_type& lhs, const T1& rhs)
    {
      return lhs.v >= rhs;
    }

    friend constexpr bool operator>=(const T1& lhs, const  ax_type& rhs)
    {
      return lhs >= rhs.v;
    }

    friend constexpr bool operator>(const ax_type& lhs, const T1& rhs)
    {
      return lhs.v > rhs;
    }

    friend constexpr bool operator>(const T1& lhs, const  ax_type& rhs)
    {
      return lhs > rhs.v;
    }

    friend std::ostream& operator<<(std::ostream& out, const ax_type& self)
    {
      out << self.get_name() << " : " << self.v;
      return out;
    }

  private:
    using T2::operator=;
  };

  namespace comparators {
    template <typename T1, typename... T_rest>
    struct _nt_compare {
      template <typename VECA_T, typename VECB_T>
      static auto __isLessthen(const VECA_T& vecA, const VECB_T& vecB)
      {
        if constexpr(sizeof...(T_rest) > 0) {
          if (T1::get(vecA) < T1::get(vecB)) {
            return true;
          } else if (T1::get(vecA) > T1::get(vecB)) {
            return false;
          }
          return _nt_compare<T_rest...>::__isLessthen(vecA, vecB);
        } else {
          return T1::get(vecA) < T1::get(vecB);
        }
      }

      template <typename VECA_T, typename VECB_T>
      static auto __isEequal(const VECA_T& vecA, const VECB_T& vecB)
      {
        if constexpr(sizeof...(T_rest) > 0) {
          if (nt::_Remove_cvref_t<T1>::get(vecA) != nt::_Remove_cvref_t<T1>::get(vecB)) {
            return false;
          }
          return _nt_compare<T_rest...>::__isEequal(vecA, vecB);
        } else {
          return nt::_Remove_cvref_t<T1>::get(vecA) == nt::_Remove_cvref_t<T1>::get(vecB);
        }
      }
    };

    template <typename... T_rest>
    auto nt_compare(T_rest&& ...)
    {
      return _nt_compare<nt::_Remove_cvref_t<T_rest>...> {};
    }

    template <typename... T_rest>
    constexpr auto lessThan(T_rest&& ...)
    {
      return [less = _nt_compare<nt::_Remove_cvref_t<T_rest>...> {}](const auto & lhs, const auto & rhs) {
        return less.__isLessthen(lhs, rhs);
      };
    }

    template <typename... T_rest>
    constexpr auto lessThan()
    {
      return [less = _nt_compare<nt::_Remove_cvref_t<T_rest>...> {}](const auto & lhs, const auto & rhs) {
        return less.__isLessthen(lhs, rhs);
      };
    }

    template <typename... T_rest>
    constexpr auto equal(T_rest&& ...)
    {
      return [less = _nt_compare<nt::_Remove_cvref_t<T_rest>...> {}](const auto & lhs, const auto & rhs) {
        return less.__isEequal(lhs, rhs);
      };
    }

    template <typename... T_rest>
    constexpr auto equal()
    {
      return [less = _nt_compare<nt::_Remove_cvref_t<T_rest>...> {}](const auto & lhs, const auto & rhs) {
        return less.__isEequal(lhs, rhs);
      };
    }
  }

  template <typename T1, typename T2>
  struct ax_type2 : T1, T2 {

    constexpr ax_type2() = default;

    using struct_maker = T2;
    using data_t = T1;
  };

  struct ax_name_container_base_const {
    static constexpr int c_struct_maker = 0;
    static constexpr int c_getter1 = 1;
    static constexpr int c_get_name = 2;
  };

  template <typename T2>
  struct ax_name_container_base : ax_name_container_base_const {
    using type = T2;

    constexpr ax_name_container_base() = default;

    template <typename T, int N>
    struct type_wrap {
      static constexpr int N_value = N;
      T val;
      using type = T;
    };

    template <typename T>
    static constexpr auto struct_maker()
    {
      return decltype(std::declval<type>()(std::declval<type_wrap<T, c_struct_maker>>())) {};
    }

    template <typename Data_T>
    using base_t = typename decltype(struct_maker<Data_T>())::type;

    template <typename T>
    static constexpr decltype(auto) get(T& t)
    {
      using getter1 = decltype(std::declval<type>()(std::declval<type_wrap<T&, c_getter1>>()));
      return getter1::get(t);
    }

    template <typename T>
    static constexpr decltype(auto) get(const T& t)
    {
      using getter1 = decltype(std::declval<type>()(std::declval<type_wrap<const T&, c_getter1>>()));
      return getter1::get(t);
    }
    template <typename T>
    static constexpr decltype(auto) get_value(const T& t)
    {
      return ax_name_container_base::get(t).v;
    }

    template <typename T>
    static constexpr decltype(auto) get_value(T& t)
    {
      return ax_name_container_base::get(t).v;
    }

    static constexpr auto get_name()
    {
      using name_getter = decltype(std::declval<type>()(std::declval<type_wrap<int, c_get_name>>()));
      return name_getter::get_name();
    }
  };

  template <typename TBase>
  struct ax_name_container : TBase {
    constexpr ax_name_container() = default;

    template <typename T>
    constexpr decltype(auto) operator()(T&& t) const
    {
      return TBase::get(t);
    }

    template <typename T>
    static constexpr decltype(auto) get_value(T& t)
    {
      return TBase::get(t).v;
    }

    template <typename T>
    static constexpr decltype(auto) get_value(const T& t)
    {
      return TBase::get(t).v;
    }

    template <typename T>
    constexpr auto operator=(T t) const
    {

      return ax_type<_Remove_cvref_t<T>, ax_name_container> {std::move(t)};
    }
  };

  template <typename... Ts>
  ax_name_container(Ts&& ...ts) -> ax_name_container<_Remove_cvref_t<Ts>...>;

  template <typename T2>
  auto constexpr get_ax_name_container(const ax_name_container<T2>& t)
  {
    return ax_name_container<T2> {};
  }

  template <typename T, typename data_T>
  struct base_maker {
    using type = typename T::template base_t<data_T>;
  };
  template <typename T, typename data_T>
  using base_maker_t = typename base_maker<T, data_T>::type;



  template <typename T>
  using ntuple_base_t = base_maker_t<_Remove_cvref_t<T>, T>;

  template <typename... T>
  struct ntuple : ntuple_base_t<T>... {
    constexpr ntuple(const ntuple& rhs) = default;
    constexpr ntuple& operator=(const ntuple& rhs) = default;
    constexpr ntuple& operator=(ntuple&& rhs) = default;
    constexpr ntuple(const T& ...t1) : ntuple_base_t<T>(t1)... {}


      constexpr ntuple() : ntuple_base_t<T>(_Remove_cvref_t<T> {})... {}

    template < typename... Ts, typename = std::enable_if_t < (sizeof...(Ts) != 1) > >
    constexpr ntuple(Ts && ...t1) : ntuple_base_t<T>(std::forward<Ts>(t1))... {}

      template <typename T2>
    decltype(auto) operator[](const ax_name_container<T2> & t)
    {
      return ax_name_container<T2>::get(*this);
    }

    friend std::ostream& operator<<(std::ostream& out, const ntuple& self)
    {
      out << "|";

      constexpr_for<0, sizeof...(T), 1>([&](auto i) {
        using  current_t = decltype(NthTypeOf<i, _Remove_cvref_t<T>...> {});
        out << " ";
        out << current_t::get(self);
        out << " |";
      });

      return out;
    }

    template <typename... ARGS>
    auto operator|(const ntuple<ARGS...>& rhs) const
    {
      return ntuple<T..., ARGS...>(
               T::get(*this)...,
               ARGS::get(rhs)...);
    }


    friend constexpr bool operator<(const ntuple& lhs, const ntuple& rhs)
    {
      constexpr auto lt = comparators::lessThan<T...>();
      return lt(lhs, rhs);
    }
    inline static constexpr std::size_t __size__ = sizeof...(T);
  };

  template <typename... Ts>
  ntuple(Ts&& ...ts) -> ntuple<_Remove_cvref_t<Ts>...>;

  template <int N, typename... ARGS>
  constexpr decltype(auto) get_nth(const nt::ntuple<ARGS...>& nt)
  {
    return nt::_Remove_cvref_t<nt::NthTypeOf<N, nt::_Remove_cvref_t<ARGS>...>>::get(nt);
  }

  template <int N, typename... ARGS>
  constexpr decltype(auto) get_nth(nt::ntuple<ARGS...>& nt)
  {
    return nt::_Remove_cvref_t<nt::NthTypeOf<N, nt::_Remove_cvref_t<ARGS>...>>::get(nt);
  }

  // Primary template for contains_type; defaults to false
  template <typename T, typename Ntuple>
  struct contains_type : std::false_type {
  };

  // Specialization for ntuple
  template <typename T, typename... ARGS>
  struct contains_type<T, ntuple<ARGS...>> : std::disjunction<std::is_same<T, ARGS>...> {
  };

  // Helper variable template
  template <typename T, typename Ntuple>
  constexpr bool contains_type_v = contains_type<T, Ntuple>::value;

}




namespace nt::comparators {

  struct on_common_args_t {

    template <typename T1, typename T2>
    constexpr static bool __comp__(T1&& t1, T2&& t2)
    {
      bool ret = true;
      constexpr_for<0, _Remove_cvref_t<T1>::__size__, 1>(
      [&](const auto i) {
        using N_th_T = _Remove_cvref_t<decltype(nt::get_nth<i>(t1))>;
        if constexpr(contains_type_v<N_th_T, _Remove_cvref_t<T2>>) {
          ret &= (N_th_T::get(t1) == N_th_T::get(t2));
        }
      });
      return ret;
    }

    template <typename T1, typename T2>
    constexpr bool operator()(T1&& t1, T2&& t2) const
    {
      return __comp__(std::forward<T1>(t1), std::forward<T2>(t2));
    }
  };
  constexpr inline on_common_args_t on_common_args;
}



namespace nt {


  template <typename... Ts>
  struct dataframe : nt::base_maker_t<nt::_Remove_cvref_t<Ts>, nt::ax_type2<std::vector<Ts>, typename Ts::struct_maker>>... {

    template <typename T2>
    decltype(auto) operator[](const nt::ax_name_container<T2>& t)
    {
      return nt::ax_name_container<T2>::get(*this);
    }

    auto operator[](size_t i)
    {
      using ret_t = nt::ntuple<_Remove_cvref_t<Ts> &...>;
      return ret_t{
        get<Ts>()[i]... };
    }

    auto operator[](size_t i) const
    {
      using ret_t = nt::ntuple<const Ts& ...>;
      return ret_t{
        get<Ts>()[i]... };
    }

    template <typename T>
    void push_back(const T& t)
    {

      [](auto...) {}(Ts::get(*this).emplace_back(Ts::get(t))...);
    }

    template <typename... T>
    void emplace_back(T&& ...t)
    {
      static_assert(sizeof...(t) == sizeof...(Ts), "\n==============missmatched amount of arguments=================\n");
      [](auto...) {}(T::get(*this).emplace_back(std::forward<T>(t))...);
    }

    template <typename T1>
    decltype(auto) get() const
    {
      return T1::get(*this);
    }

    template <typename T1>
    decltype(auto) get()
    {
      return T1::get(*this);
    }

    auto size() const
    {
      auto size = _Remove_cvref_t<NthTypeOf<0, Ts...>>::get(*this).size();
      return size;
    }

    template <int N>
    static constexpr auto get_nth_type()
    {

      return get_ax_name_container(NthTypeOf<N, _Remove_cvref_t<Ts>...> {});
    }
    friend std::ostream& operator<<(std::ostream& out, const dataframe& self)
    {
      out << "|";

      constexpr_for<0, sizeof...(Ts), 1>([&](auto ntuple_index) {
        static const auto x = self.template  get_nth_type<ntuple_index>();
        out << " ";
        out << std::setw(5) << x.get_name();
        out << " |";
      });

      out << "\n";
      out << "|";
      constexpr_for<0, sizeof...(Ts), 1>([&](auto i) {
        out << std::setw(5) << "-------|";
      });
      out << "\n";
      auto size = self.size();
      for (int i = 0; i < size; ++i) {
        auto current_element = self[i];
        out << "|";
        constexpr_for<0, sizeof...(Ts), 1>([&](auto ntuple_index) {
          static const  auto x = self.template get_nth_type<ntuple_index>();
          out << " ";
          out << std::setw(5) << x.get(current_element).v;
          out << " |";
        });
        out << "\n";
      }

      return out;
    }
  };

  template <typename... Ts>
  dataframe(Ts&& ...ts) -> dataframe<_Remove_cvref_t<Ts>...>;

  template <typename T>
  struct dataframe_maker {
  };

  template <typename... T>
  struct dataframe_maker<ntuple<T...>> {
    using type = dataframe<_Remove_cvref_t<T>...>;
  };

  template <typename F>
  auto fill_dataframe(int index, F&& f)
  {
    typename dataframe_maker<decltype(f(0))>::type ret;
    for (int i = 0; i < index; ++i) {
      ret.push_back(f(i));
    }
    return ret;
  }

  template <typename NT_T, typename FUNT>
  void ntuple_for_each(const NT_T& ntuple, FUNT&& fun)
  {
    nt::constexpr_for <
    0,
    nt::_Remove_cvref_t<decltype(ntuple)>::__size__,
    1 > (
    [&](auto j) {
      fun(
        nt::get_nth<j>(ntuple)
      );
    }
    );
  }
}


namespace nt::algorithms {



  struct identity_t {
    template <typename T>
    constexpr T&& operator()(T&& t) const
    {
      return t;
    }
  };
  template <typename VEC, typename FUNC_T = identity_t>
  auto sum(const VEC& vec, FUNC_T && proj = identity_t {})
  {
    decltype(proj(vec[0]))  ret{};
    for (auto&& e : vec) {
      ret = ret + proj(e);
    }
    return ret;
  }


  template <typename VEC, typename FUNC_T>
  auto add_column(const VEC& vec, FUNC_T&& func)
  {
    std::vector < decltype(vec[0] | func(vec[0])) > ret;
    ret.reserve(vec.size());
    for (const auto& e : vec) {
      ret.push_back(e | func(e));
    }
    return ret;
  }

  template <typename FUNC_T>
  auto fill_vector(size_t entries, FUNC_T&& func)
  {
    std::vector<decltype(func(size_t(0)))> ret;
    ret.reserve(entries);
    for (size_t i = 0; i < entries; ++i) {
      ret.push_back(func(i));
    }
    return ret;
  }

  template <typename T0, typename T1, typename T2, typename Comparision_T, typename projecttion_t>
  void join_vectors_r(T0& ret, const T1& t1, const T2& t2, Comparision_T comp, projecttion_t project)
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

  template <typename T1, typename T2, typename Comparision_T, typename projecttion_t>
  auto join_vectors(const T1& t1, const T2& t2, Comparision_T comp, projecttion_t project)
  {
    std::vector<decltype(project(t1[0], t2[0]))> ret;
    join_vectors_r(ret, t1, t2, comp, project);
    return ret;
  }

  template <typename CONTAINER_T>
  void sort(CONTAINER_T& container)
  {
    std::sort(container.begin(), container.end());
  }

  template <typename CONTAINER_T, typename COMP_T>
  void sort(CONTAINER_T& container, COMP_T&& comp)
  {
    std::sort(container.begin(), container.end(), comp);
  }

  template <typename CONTAINER_T, typename OP_T>
  auto count_if(const  CONTAINER_T& container, OP_T op)
  {
    int i = 0;
    for (const auto& e : container)
      if (op(e))
        ++i;
    return i;
  }

  template <typename VEC_T, typename FILTER_T>
  void filter(VEC_T& vec, FILTER_T&& f)
  {
    auto removeIt = std::remove_if(vec.begin(), vec.end(), [&](auto&& e) {return !f(e); });
    vec.erase(removeIt, vec.end());

  }

  template <typename VEC_T, typename FILTER_T>
  auto filter_copy(const VEC_T& vec, FILTER_T&& f)
  {
    VEC_T ret;
    ret.reserve(vec.size());
    for (const auto& e : vec) {
      if (f(e)) {
        ret.push_back(e);
      }
    }
    return ret;
  }
}



namespace nt::algorithms {



  template <typename... T>
  struct __group {
    template <typename VEC_T, typename FUNC_T>
    static auto apply_append(const std::vector<VEC_T>& vec, FUNC_T&& fun)
    {
      auto fun_c = [&](const auto & rng) {
        auto tail = rng.begin();
        return nt::ntuple<T...>(T::get(*tail)...) | fun(rng);
      };
      return __apply__internal__(vec, fun_c);
    }

    template <typename VEC_T, typename FUNC_T>
    static auto apply(const std::vector<VEC_T>& vec, FUNC_T&& fun)
    {
      auto fun_c = [&](const auto & rng) {
        return fun(rng);
      };
      return __apply__internal__(vec, fun_c);
    }

  private:

    template <typename VEC_T, typename FUNC_T>
    static auto create_empty_vector()
    {
      return std::vector< decltype(std::declval<FUNC_T>()(std::declval<const std::vector<VEC_T>& >())) > {};
    }

    template <typename VEC_T, typename FUNC_T>
    static auto __apply__internal__(const std::vector<VEC_T>& vec, FUNC_T&& fun_c)
    {
      static constexpr auto lt = nt::comparators::lessThan<T...>();
      static constexpr auto eq = nt::comparators::equal<T...>();
      auto ret = create_empty_vector<VEC_T, FUNC_T>();



      if (vec.empty()) {
        return ret;
      }
      auto min_element = vec[0];
      auto max_element = vec[0];

      for (const auto& e : vec) {
        if (lt(e, min_element)) {
          min_element = e;
        }

        if (lt(max_element, e)) {
          max_element = e;
        }
      }

      if (eq(min_element, max_element)) {
        ret.push_back(
          fun_c(vec)
        );
        return ret;
      }

      std::vector<VEC_T> buff{};
      buff.reserve(vec.size());



      auto process = [&]() {
        buff.clear();
        VEC_T next_min_element{};
        next_min_element = max_element;

        for (const auto& e : vec) {
          if (eq(min_element, e)) {
            buff.push_back(e);
          } else if (lt(min_element, e) && lt(e, next_min_element)) {
            next_min_element = e;
          }

        }

        min_element = next_min_element;


        ret.push_back(fun_c(buff));
      };

      while (lt(min_element, max_element)) {
        process();

      }

      process();


      return ret;
    }
  };

  template <typename... ARGS>
  auto group(ARGS...)
  {
    return __group<nt::_Remove_cvref_t<ARGS>...> {};
  }


  template <typename T>
  constexpr auto get_default_element(T&& t)
  {
    return nt::_Remove_cvref_t<decltype(t[0])> {};
  }
}





#define ax_maker(name_) []() constexpr {                                          \
    auto struct_maker_template_lambda = [](auto e) constexpr {                      \
      using ARG_T = decltype(e);                                                    \
      if constexpr (e.N_value == nt::ax_name_container_base_const::c_struct_maker)  \
      {                                                                             \
        if constexpr (!std::is_reference_v<typename ARG_T::type>)                   \
        {                                                                           \
          struct Zt##name_                                                          \
          {                                                                         \
            constexpr Zt##name_() {}                                                          \
            constexpr Zt##name_(const decltype(e.val) &e_) : name_(e_) {}                     \
            constexpr Zt##name_(decltype(e.val) &e_) : name_(e_) {}                           \
            constexpr Zt##name_(decltype(e.val) &&e_) : name_(std::move(e_)) {}               \
            decltype(e.val) name_;                                                  \
            decltype(e.val) value() const                                           \
            {                                                                       \
              return name_;                                                         \
            }                                                                       \
          };                                                                        \
          return nt::type_container<Zt##name_>{};                                   \
        }                                                                           \
        else                                                                        \
        {                                                                           \
          struct Zt##name_                                                          \
          {                                                                         \
            Zt##name_(decltype(e.val) e_) : name_(e_) {}                            \
            decltype(e.val) name_;                                                  \
            decltype(e.val) value() const                                           \
            {                                                                       \
              return name_;                                                         \
            }                                                                       \
          };                                                                        \
          return nt::type_container<Zt##name_>{};                                   \
        }                                                                           \
      }                                                                             \
      else if constexpr (e.N_value == nt::ax_name_container_base_const::c_getter1)  \
      {                                                                             \
        struct getter_t                                                             \
        {                                                                           \
          static auto &get(decltype(e.val) x)                             \
          {                                                                         \
            return x.name_;                                                         \
          }                                                                         \
        };                                                                          \
        return getter_t{};                                                          \
      }                                                                             \
      else if constexpr (e.N_value == nt::ax_name_container_base_const::c_get_name) \
      {                                                                             \
        struct name_getter_t                                                        \
        {                                                                           \
          static std::string get_name()                                          \
          {                                                                         \
            return #name_;                                                          \
          }                                                                         \
        };                                                                          \
        return name_getter_t{};                                                     \
      }                                                                             \
    };                                                                              \
    return nt::ax_name_container<                                                   \
           nt::ax_name_container_base<decltype(struct_maker_template_lambda)>>{};      \
  }()

#define ax_new_axis(name_, value) static constexpr inline auto name_ = (ax_maker(name_) = value)

#define ax_new_axis_t(name_, value)                                  \
  auto __internal__##name_ = [] { return ax_maker(name_) = value; }; \
  using name_ = decltype(__internal__##name_())


#define nt_lessthan(...) [](){ using T = decltype(             \
                                           nt::comparators::nt_compare( __VA_ARGS__ ) ); \
  static constexpr T t{};                 \
  return [&](const auto& x,const auto &y) {return t.__isLessthen(x,y);}; }()

#define nt_equal(...) [](){ using T = decltype(             \
                                                              nt::comparators::nt_compare( __VA_ARGS__ ) ); \
  static constexpr T t{};                 \
  return [&](const auto& x,const auto &y) {return t.__isEequal(x,y);}; }()



#define nt_group(...) [](){ using T = decltype(             \
                                                              nt::algorithms::group( __VA_ARGS__ ) ); \
  static constexpr T t{};                 \
  return t; }()



// ax_new_axis(name_1231, 123);

#define __nt_new_axis_core(name_)                         \
  struct zt##name_                                        \
  {                                                       \
    template <typename T>                                 \
    struct type_wrap                                      \
    {                                                     \
      constexpr type_wrap() {}                                      \
      template <typename T1>                              \
      constexpr type_wrap(T1 &&e_) : name_(std::forward<T1>(e_)) {} \
      T name_;                                            \
    };                                                    \
    template <typename Data_T>                            \
    using base_t = type_wrap<Data_T>;                     \
    static auto get_name()                                \
    {                                                     \
      return #name_;                                      \
    }                                                     \
    template <typename T>                                 \
    static constexpr auto &get(T &t)                      \
    {                                                     \
      return t.name_;                                     \
    }                                                     \
    template <typename T>                                 \
    static constexpr const auto &get(const T &t)          \
    {                                                     \
      return t.name_;                                     \
    }                                                     \
  }

#define __nt_new_axis(qualifier, name_, value) \
  namespace __nt                               \
  {                                            \
    __nt_new_axis_core(name_);                 \
  }                                            \
  qualifier name_ = (nt::ax_name_container<__nt::zt##name_>{} = value)

#define nt_new_axis(name_, value) __nt_new_axis(static constexpr inline auto, name_, value)
#define nt_new_axis_c(name_, value) __nt_new_axis(static const inline auto, name_, value)

#define nt_new_axis_t(name_, value) \
  namespace __nt                    \
  {                                 \
    __nt_new_axis_core(name_);      \
  }                                 \
  using name_ = decltype(nt::ax_name_container<__nt::zt##name_>{} = value)

#define nt_new_name(name_)     \
  namespace __nt               \
  {                            \
    __nt_new_axis_core(name_); \
  }                            \
  static constexpr inline auto name_ = nt::ax_name_container<__nt::zt##name_> {}

#define nt_new_name_t(name_)   \
  namespace __nt               \
  {                            \
    __nt_new_axis_core(name_); \
  }                            \
  using name_ = nt::ax_name_container<__nt::zt##name_>





