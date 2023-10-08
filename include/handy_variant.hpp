#ifndef __HANDY_VARIANT
#define __HANDY_VARIANT
#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace handy_variant
{
    namespace
    {
        template <typename T>
        constexpr size_t get_index_in(size_t index)
        {
            return 0;
        };

        template <typename T, typename L, typename... Types>
        constexpr size_t get_index_in(size_t index)
        {
            if constexpr (std::is_same<L, T>{})
                return index;
            else
                return get_index_in<T, Types...>(index + 1);
        };

    } // namespace

    namespace
    {
        template <typename L, typename T,
                  bool is_convertible = std::is_convertible<L, T>::value>
        struct _basic_cast
        {
            static T vl(const L &v)
            {
                return T{};
            };
        };

        /// static_cast is available.
        template <typename L, typename T>
        struct _basic_cast<L, T, true>
        {
            static T vl(const L &v)
            {
                return static_cast<T>(v);
            };
        };

        // static_cast is NOT available.
        template <typename L, typename T>
        struct _basic_cast<L, T, false>
        {
            static T vl(const L &v)
            {
                throw std::invalid_argument(std::string("can't static_cast from ") //
                                            + typeid(L).name() + " to " + typeid(T).name());
            };
        };

        // specification for casting std::string
        // to string
        template <>
        struct _basic_cast<int, std::string, false>
        {
            static std::string vl(const int &v) { return std::to_string(v); };
        };
        template <>
        struct _basic_cast<long, std::string, false>
        {
            static std::string vl(const long &v) { return std::to_string(v); };
        };
        template <>
        struct _basic_cast<double, std::string, false>
        {
            static std::string vl(const double &v) { return std::to_string(v); };
        };
        template <>
        struct _basic_cast<float, std::string, false>
        {
            static std::string vl(const float &v) { return std::to_string(v); };
        };
        template <>
        struct _basic_cast<bool, std::string, false>
        {
            static std::string vl(const bool &v) { return v ? "true" : "false"; };
        };

        // from string
        template <>
        struct _basic_cast<std::string, int, false>
        {
            static int vl(const std::string &v) { return std::stoi(v); };
        };
        template <>
        struct _basic_cast<std::string, long, false>
        {
            static long vl(const std::string &v) { return std::stol(v); };
        };
        template <>
        struct _basic_cast<std::string, double, false>
        {
            static double vl(const std::string &v) { return std::stof(v); };
        };
        template <>
        struct _basic_cast<std::string, float, false>
        {
            static float vl(const std::string &v) { return std::stod(v); };
        };
        template <>
        struct _basic_cast<std::string, bool, false>
        {
            static bool vl(const std::string &v) { return v == "true"; };
        };
    } // namespace

    /// @brief Get the index corresponding to the specified type T
    /// @tparam T
    /// @tparam ...Types
    /// @param t Specifying the type std::variant
    /// @return The index for type T
    template <typename T, typename... Types>
    static constexpr size_t index_of(const std::variant<Types...> &t)
    {
        static_assert(!std::conjunction<std::is_same<T, Types>...>::value,
                      "There is no matching type among the variant type candidates.");
        return get_index_in<T, Types...>(0);
    };

    /// @brief Convert to type T from std::variant
    /// @tparam T convert to type T
    /// @tparam ...Types variant types
    /// @param t Value of std::variant type to be converted
    /// @return converted value
    /// @exception std::invalid_argument exception is raised if type conversion is not possible
    template <typename T, typename... Types>
    T variant_cast(const std::variant<Types...> &t)
    {
        static_assert(!std::conjunction<std::is_same<T, Types>...>::value,
                      "There is no matching type among the variant type candidates.");
        if (std::holds_alternative<T>(t))
            return std::get<T>(t);

        return std::visit(
            [](const auto &v) -> T
            {
                // const auto & -> auto
                return _basic_cast<typename std::remove_const<typename std::remove_reference<decltype(v)>::type>::type, T>::vl(v);
            },
            t);
    };

    /// @brief Convert to type T from std::variant
    /// @tparam T convert to type T
    /// @tparam ...Types variant types
    /// @param t Value of std::variant type to be converted
    /// @param default_value The value to be returned if conversion is not possible.
    /// @return converted value
    template <typename T, typename... Types>
    T variant_cast(const std::variant<Types...> &t, const T &default_value) noexcept
    {
        try
        {
            return variant_cast<T, Types...>(t);
        }
        catch (std::exception)
        {
            return default_value;
        }
    }

    template <typename... dstTypes, typename... srcTypes>
    std::variant<dstTypes...> variant_to_variant(const std::variant<srcTypes...> &t)
    {
        return std::visit(
            [](const auto &v) -> std::variant<dstTypes...>
            {
                using v_type = typename std::remove_const<typename std::remove_reference<decltype(v)>::type>::type;
                if constexpr (std::disjunction<std::is_same<v_type, dstTypes>...>::value)
                    return v;
                else
                    throw std::invalid_argument("Incompatible type specified.");
            },
            t);
    };

    template <typename... srcTypes, typename... dstTypes>
    void variant_to_variant(std::variant<dstTypes...> &dst, const std::variant<srcTypes...> &t)
    {
        dst = std::visit(
            [](const auto &v) -> std::variant<dstTypes...>
            {
                using v_type = typename std::remove_const<typename std::remove_reference<decltype(v)>::type>::type;
                if constexpr (std::disjunction<std::is_same<v_type, dstTypes>...>::value)
                    return v;
                else
                    throw std::invalid_argument("Incompatible type specified.");
            },
            t);
    };

    /// @brief Maps with type information
    template <typename... Types>
    struct variant_map
    {
        typedef std::variant<Types...> variant_type;
        std::unordered_map<std::string, variant_type> values;

        variant_map() : values(){};
        variant_map(const std::unordered_map<std::string, variant_type> &l) : values(l){};

        int count(const std::string &key) const noexcept
        {
            return values.count(key);
        };

        bool has(const std::string &key) const noexcept
        {
            return values.count(key) > 0;
        };

        template <typename T>
        bool is_hold_as(const std::string &key) const noexcept
        {
            if (values.count(key) <= 0)
                return false;
            if (std::holds_alternative<T>(values.at(key)))
                return true;
            return false;
        };

        template <typename T>
        const T &get(const std::string &key, const T &default_value) const noexcept
        {
            if (values.count(key) <= 0)
                return default_value;
            if (std::holds_alternative<T>(values.at(key)))
                return std::get<T>(values.at(key));
            return default_value;
        };

        template <typename T>
        T cast_get(const std::string &key, const T &default_value) const noexcept
        {
            if (values.count(key) <= 0)
                return default_value;
            if (std::holds_alternative<T>(values.at(key)))
                return std::move(std::get<T>(values.at(key)));
            return std::move(variant_cast<T>(values.at(key), default_value));
        };

        template <typename T>
        T cast_get(const std::string &key) const
        {
            if (values.count(key) <= 0)
                throw std::invalid_argument("No such key as :" + key);
            if (std::holds_alternative<T>(values.at(key)))
                return std::move(std::get<T>(values.at(key)));
            return std::move(variant_cast<T>(values.at(key)));
        };

        template <typename T>
        void set(const std::string &key, const T &v) noexcept
        {
            values[key] = v;
        };

        template <typename T>
        auto emplace(const std::string &key, const T &v) noexcept
        {
            return values.emplace(key, v);
        };

        size_t index_at(const std::string &key) const
        {
            return values.at(key).index();
        };

        template <typename T>
        static constexpr size_t index_of()
        {
            static_assert(!std::conjunction<std::is_same<T, Types>...>::value,
                          "There is no matching type among the variant type candidates.");
            return get_index_in<T, Types...>(0);
        };

        static constexpr size_t types_count()
        {
            return std::variant_size<variant_type>::value;
        };

        auto operator[](const std::string &name) const
        {
            return values[name];
        };
    };

} // namespace appkit
#endif