/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <algorithm>
#include <sstream>

#include <array>
#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

namespace Belle2 {
  namespace ARICHTools {

    /**
     * @brief The ModuleID_t class is a intermediate object generated to contain
     * configurations in a sorted fashion.
     */
    class ModuleID_t {
    public:
      // range definitions
      /**
        * @brief m_gValidSectors is a array containing allowed sector ids
        */
      static constexpr auto m_gValidSectors =
      std::array<uint8_t, 4>({{'A', 'B', 'C', 'D'}});

      /**
        * @brief m_gMaxChannel number of maximum channel starting from 0
        */
      static constexpr auto m_gMaxChannel = int8_t{36};

      /**
       * @brief isValidChannel defines the range of valid channels.
       * @param channel
       * @return true if channel id/number is in the valid range otherwise false.
       */
      static constexpr auto isValidChannel(const int channel) noexcept -> bool
      {
        return (channel >= 0) && (channel <= m_gMaxChannel);
      }

      static auto isValidSector(const uint8_t sector) noexcept -> bool
      {
        return std::find(m_gValidSectors.begin(), m_gValidSectors.end(), sector) !=
               m_gValidSectors.end();
      }

    public:
      /**
       * @brief ModuleID_t is the default ctor that construct an id out of a sector
       * id and channel number
       * @param sector
       * @param channel
       * @see ModuleID_t::m_gValidSectors
       */
      ModuleID_t(const int8_t sector = '0', const int channel = 0) noexcept
        : m_ID((sector << m_gMemberSize) | channel) {}

      /**
       * @brief ModuleID_t used to construct from a given integer that has a valid
       * module ID. Note: An invalid ID causes an undefined behaviour!
       * @param id is a valid module ID
       */
      ModuleID_t(const uint16_t id) noexcept : m_ID(id) {}

      /**
       * @brief isValidID check is constructed id is valid
       * @return true if this ModuleID_t is valid, otherwise false.
       */
      inline auto isValidID() const noexcept -> bool
      {
        return ModuleID_t::isValidSector(this->getSector()) &&
               ModuleID_t::isValidChannel(this->getChannel());
      }

      /**
       * @brief getSector returns the definded sectors.
       * @return sector id
       * @see ModuleID_t::m_gValidSectors
       */
      inline auto getSector() const noexcept -> int8_t
      {
        return ((m_ID >> m_gMemberSize) & m_gMemberMask);
      }

      /**
       * @brief getChannel return the defined channel
       * @return channel id/number
       * @see ModuleID_t::isValidChannel()
       */
      inline auto getChannel() const noexcept -> int8_t
      {
        return (m_ID & m_gMemberMask);
      }

      /**
       * @brief getNumbering returns the channel number Ax = 1+x; Bx = 36 + x; Cx =
       * 72 + x; Dx = 108 + x; Note: an invalid id return an invalid number!!!
       * @return channel number increamened by an offset determined by section ID.
       * @see isValidID()
       */
      inline auto getNumbering() const noexcept -> int
      {
        return (this->getSector() - m_gValidSectors.front()) * m_gMaxChannel +
               this->getChannel();
      }

      /**
       * @brief operator << to print the id using any std::ostream implementation...
       * @param rStream
       * @param id
       */
      friend inline auto operator<<(std::ostream& rStream, const ModuleID_t id)
      -> std::ostream& {
        rStream << id.getSector() << static_cast<int>(id.getChannel());
        return rStream;
      }

      /**
       * comparison operator==
       * @note if using c++17 replace thi one with <=> operator
       */
      inline auto operator==(const ModuleID_t& rOther) const noexcept -> bool
      {
        return m_ID == rOther.m_ID;
      }

      /**
       * comparison operator!=
       * @note if using c++17 replace thi one with <=> operator
       */
      inline auto operator!=(const ModuleID_t& rOther) const noexcept -> bool
      {
        return m_ID != rOther.m_ID;
      }

      /**
       * comparison operator<
       * @note if using c++17 replace thi one with <=> operator
       */
      inline auto operator<(const ModuleID_t& rOther) const noexcept -> bool
      {
        return m_ID < rOther.m_ID;
      }

      /**
       * comparison operator<=
       * @note if using c++17 replace thi one with <=> operator
       */
      inline auto operator<=(const ModuleID_t& rOther) const noexcept -> bool
      {
        return m_ID <= rOther.m_ID;
      }

      /**
       * comparison operator>
       * @note if using c++17 replace thi one with <=> operator
       */
      inline auto operator>(const ModuleID_t& rOther) const noexcept -> bool
      {
        return m_ID > rOther.m_ID;
      }

      /**
       * comparison operator>=
       * @note if using c++17 replace thi one with <=> operator
       */
      inline auto operator>=(const ModuleID_t& rOther) const noexcept -> bool
      {
        return m_ID >= rOther.m_ID;
      }

      /**
       * precrement operator ++X
       * @note does not check the module range!!! overflow is allowed!!!
       */
      inline auto operator++() noexcept -> ModuleID_t&
      {
        m_ID = this->getChannel() >= m_gMaxChannel
               ? (((this->getSector() + 1) << m_gMemberSize) | 1)
               : m_ID + 1;
        return *this;
      }

      /**
       * postcrement operator X++
       * @note does not check the module range!!! overflow is allowed!!!
       */
      inline auto operator++(int)noexcept -> ModuleID_t
      {
        auto tmp = ModuleID_t(m_ID);
        this->operator++();
        return tmp;
      }

    private:
      /**
       * @brief m_gMemberSize size of each sub ids
       */
      static constexpr auto m_gMemberSize = sizeof(int8_t) * 8;
      /**
       * @brief m_gMemberMask is a mask to get the desired sub id info.
       */
      static constexpr auto m_gMemberMask = 0xff;

      /**
       * @brief m_ID contains the unique sector and channel ids.
       */
      uint16_t m_ID;
    };
    constexpr std::array<uint8_t, 4> ModuleID_t::m_gValidSectors;

    namespace PrivateHelperClasses {
// helper classes for the defined conversions

      /**
       * Generic template for TokenCast<T>
       */
      template <typename Desired_t> struct TokenCast;

      /**
       * Integer specialization of TokenCast<T>
       */
      template <> struct TokenCast<int> {
        /**
         * @brief isValidChar defines characters that are allowed for the convertion
         * @param character
         * @return true if its allowed for convertion
         */
        inline auto isValidChar(const unsigned char character) const noexcept
        -> bool
        {
          return std::isdigit(character);
        }

        /**
         * @brief operator () converts given token to an integer.
         * @param rToken
         * @throw invalid_argument in case the given string is invalid for
         * convertion
         * @throw std::out_of_range if the given number is smaller/greater than a
         * integer.
         */
        inline auto operator()(const std::string& rToken) const -> int
        {
          return std::stoi(rToken);
        }
      };

      /**
       * Double specialization of TokenCast<T> the decimal point is set to '.'!
       * Note: the spelication on locale is unclear need Basf2 facet defs!
       * @todo check with framework developer on set facet that defines the decimal
       * point...
       */
      template <> struct TokenCast<double> {
        /**
         * @brief isValidChar defines characters that are allowed for the convertion
         * @param character
         * @return true if its allowed for convertion
         */
        inline auto isValidChar(const unsigned char character) const noexcept
        -> bool
        {
          return std::isdigit(character) || (static_cast<unsigned char>('.') == character);
        }

        /**
         * @brief operator () converts given token to an integer.
         * @param rToken
         * @throw invalid_argument in case the given string is invalid for
         * convertion
         * @throw std::out_of_range if the given number is smaller/greater than a
         * double.
         */
        inline auto operator()(const std::string& rToken) const -> double
        {
          return std::stod(rToken);
        }
      };

      /**
       * ModuleID_t specialization of TokenCast<T> the decimal point is set to '.'!
       */
      template <> struct TokenCast<ModuleID_t> {
        /**
         * @brief isValidChar defines characters that are allowed for the convertion
         * @param character
         * @return true if its allowed for convertion
         */
        inline auto isValidChar(const unsigned char character) const noexcept
        -> bool
        {
          return std::isdigit(character) || ModuleID_t::isValidSector(character);
        }

        /**
         * @brief operator () converts given token to an integer.
         * @param rToken
         * @throw invalid_argument in case the given string is invalid for
         * convertion
         * @throw std::out_of_range if the given number is smaller/greater than a
         * double.
         */
        inline auto operator()(const std::string& rToken) const -> ModuleID_t
        {
          const auto begin =
            std::find_if(rToken.begin(), rToken.end(),
          [](const char c) { return !std::isspace(c); });
          const auto end = std::find_if(begin, rToken.end(),
          [](const char c) { return std::isspace(c); });

          if ((begin + 1 == end) || !std::all_of(begin, end, [this](const char c) {
          return this->isValidChar(c);
          }))
          throw std::invalid_argument("Invalid argiment for module cast, got: '" +
                                      rToken + "'!");

          const auto chID = std::stoi(std::string(begin + 1, end));
          if (ModuleID_t::isValidSector(*begin) && ModuleID_t::isValidChannel(chID))
            return ModuleID_t(*begin, chID);

          throw std::out_of_range("Module ID out of range. got: '" + rToken + "' !");
        }
      };
    } // end namespace PrivateHelperClasses

    /**
     * @brief The StringToVector class converts a given string to the
     * std::vector<T>.
     * The input string is allowed to contain white spaces and the specific types.
     */
    class StringToVector {
    public:
      /**
       * @brief m_gRangeOperator is a std::wstring  containing the range symbold
       */
      static const std::string m_gRangeOperator;

    public:
      /**
       * @brief convert<T> converts the given string in to a std::vector<T> elements
       * seperated by the given delimiter
       * @param rLine tokens seperated by the delimiter
       * @param delim delimiter character
       * @return std::vector<T>
       * @throw std::runtime_error() if
       *   - rLine contains other characters than delimiter, white space, digits or
       * allowed characters defined by PrivateHelperClasses::TokenCast<T>
       * @see PrivateHelperClasses::TokenCast<T> and its specializations
       */
      template <typename T>
      static inline auto convert(const std::string& rLine, const char delim = ' ')
      -> std::vector<T> {
        const auto cast = PrivateHelperClasses::TokenCast<T>();
        // check if line only contains white space, numbers and delimiter char.
        if (!std::all_of(rLine.begin(), rLine.end(),
        [&cast, delim](const unsigned char c)
      {
        return std::isdigit(c) || std::isspace(c) ||
          (c == delim) || cast.isValidChar(c);
        }))
        throw std::runtime_error("Detected invalid character in '" + rLine +
        "'!");

        auto iss = std::istringstream(rLine);
        auto retval = std::vector<T>();
        auto token = std::string();

        // convert string to number and add it to vector...
        while (std::getline(iss, token, delim))
          try {
            retval.emplace_back(cast(token));
          } catch (const std::invalid_argument& rErr)
          {
            throw std::runtime_error("Invalid token, got:'" + token + "'! " +
            rErr.what());
          } catch (const std::out_of_range& rErr)
          {
            // if the values exceeds the requested type
            throw std::runtime_error("Conversion out of range, got: '" + token +
            "'! " + rErr.what());
          }

        return retval;
      }

      /**
       * @brief parse<T> expands the given string and evaluats defined operators.
       * Currently only the range operator is defined and used.
       * @param rLine
       * @param delim
       * @return the evaluated std::string.
       * @throw std::runtime_error() if
       *   - invalid argument for during the operator parsing
       *   - out of range convertion, ie casting 0xffff into int8_t
       */
      template <typename T>
      static inline auto parse(const std::string& rLine, const char delim = ' ')
      -> std::string {
        auto out = std::stringstream();
        auto iss = std::istringstream(rLine);
        auto token = std::string();

        if (!std::getline(iss, token, delim))
          return std::string();

        if (std::search(token.begin(), token.end(), m_gRangeOperator.begin(),
        m_gRangeOperator.end()) != token.end())
        {
          StringToVector::expand<T>(out, delim, token);
        } else {
          out << token;
        }

        // convert string to number and add it to vector...
        while (std::getline(iss, token, delim))
          try {
            if (std::search(token.begin(), token.end(), m_gRangeOperator.begin(),
            m_gRangeOperator.end()) != token.end())
            {
              out << ',';
              StringToVector::expand<T>(out, delim, token);
            } else {
              out << delim << token;
            }
          } catch (const std::invalid_argument& rErr)
          {
            throw std::runtime_error("Invalid token, got:'" + token + "'! " +
            rErr.what());
          } catch (const std::out_of_range& rErr)
          {
            // if the values exceeds the requested type
            throw std::runtime_error("Conversion out of range, got: '" + token +
            "'! " + rErr.what());
          }
        return out.str();
      }

    private:
      /**
       * @brief expand<T> applies the given range and fills the given std::ostream
       * with each element in range seperated by the delimiter char
       * @param rStream
       * @param delim
       * @param rToken
       * @throw std::runtime_error if invalid range definition, ie A to B with A > B
       * @note the precrement operator of T has to be defined, ie T::operator++()
       */
      template <typename T>
      static inline auto expand(std::ostream& rStream, const char delim,
                                const std::string& rToken) -> std::ostream& {
        auto itPos = std::search(rToken.begin(), rToken.end(),
        m_gRangeOperator.begin(), m_gRangeOperator.end());
        const auto cast = PrivateHelperClasses::TokenCast<T>();
        auto lhs = cast(std::string(rToken.begin(), itPos));
        std::advance(itPos, m_gRangeOperator.size());
        auto rhs = cast(std::string(itPos, rToken.end()));

        if (lhs >= rhs)
          throw std::runtime_error(
            "Invalid expansion! lhs ist greater then rhs, got: '" + rToken +
            "'!");

        rStream << lhs;
        for (++lhs; lhs <= rhs; ++lhs)
          rStream << delim << lhs;
        return rStream;
      }
    };
    const std::string StringToVector::m_gRangeOperator("～");

    /**
     * @brief getDeadCutList converts a list of dead channel stored in line. the
     * returned std::vector<int> contains the channels with a given offset defined
     * by the chipID.
     * @param chipID in range of ['A' = 0, 'B' = 36, 'C' = 2*36, 'D'=3*36]
     * (chipID=offset)
     * @param line
     * @return std::vector<int> of channel with a denined offset.
     */
    auto getDeadCutList(const char chipID, const std::string& line)
    -> std::vector<int> {
      auto ids = StringToVector::convert<int>(line, ','); // can throw
      for (auto& rID : ids)
        rID = ModuleID_t(chipID, rID).getNumbering();

      return ids;
    }

    /**
     * @brief remove_nondigit removes all non digit char from a string reference!!!
     * @note auto x = remove_nondigit("hello world") will cause a seg fault at best
     * @param s is a non-const string reference
     * @return reference back...
     * @todo change 'std::string& s' to 'std::string&& s' and just 'return s'. the
     * return type has to be changed to 'std::string', this is also behaviour
     * altering, thus need to be double checked if the user code allows for
     * modification! or just change the return value to void() and define s as
     * in/out parameter.
     */
    std::string& remove_nondigit(std::string& s)
    {
      // changed to unsigned char see documention of std::isdigit in <cctype>
      s.erase(remove_if(s.begin(), s.end(),
      [](const unsigned char& c) { return !std::isdigit(c); }),
      s.end());
      return s;
    }

    /**
     * @brief remove_chars_if_not
     * @param s
     * @param allowed
     * @return
     * @note same problems as @see remove_nondigit(std::string &s)
     */
    std::string& remove_chars_if_not(std::string& s, const std::string& allowed)
    {
      s.erase(remove_if(s.begin(), s.end(),
      [&allowed](const char& c) {
        return allowed.find(c) == std::string::npos;
      }),
      s.end());
      return s;
    }

  } // end namepace ARICHTools
} // end namepace Belle2
