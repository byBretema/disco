#pragma once

/* alt_cpp (ac) - v0.01

    An alternative take on cpp.
    Basically some aliases and helpers.

    Also a bunch of code that I found myself repeating between project.

    No warranty implied, use at your own risk.

    =============================================
    ! How to include
    =============================================

    -- Classic header-only stuff, add this:

    #define ALT_CPP_IMPLEMENTATION

    -- Before you include this file in *one* C++ file to create the
    implementation, something like this:

    #include ...
    #include ...
    #define ALT_CPP_IMPLEMENTATION
    #include "alt.hpp"

    =============================================
    ! Define-Based options:
    =============================================

    -- If you use fmt-lib, 'alt' will include basic fmt header files and
    expose, basic log methods: ac_info/warn/err/debug("", ...),
    this will also undefine 'ALT_CPP_USE_FAKE_FMT'

    #define ALT_CPP_INCLUDE_FMT

    -- If you use glm-lib, 'alt' will include basic glm header files

    #define ALT_CPP_INCLUDE_GLM

    -- Use a naïve fmt-like custom implemenation (will be disabled if
    'ALT_CPP_INCLUDE_FMT' is present)

    #define ALT_CPP_USE_FAKE_FMT
*/


//=========================================================
//== INCLUDES
//=========================================================

#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>

#include <array>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <algorithm>
#include <memory>
// #include <functional>

#ifdef ALT_CPP_INCLUDE_FMT
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#endif

#ifdef ALT_CPP_INCLUDE_GLM
// #define GLM_FORCE_SSE
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/string_cast.hpp> // Before of 'GLM_FORCE_INLINE'
#define GLM_FORCE_INLINE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtx/vector_angle.hpp>
#define glmstr(x) glm::to_string(x)
#endif


//=========================================================
//== CONCAT
//=========================================================

#define __AC_CONCAT2(l, r) l##r
#define __AC_CONCAT1(l, r) __AC_CONCAT2(l, r)
#define AC_CONCAT(l, r) __AC_CONCAT1(l, r)


//=========================================================
//== DEFER
//=========================================================


#define ac_defer(fn) const auto AC_CONCAT(defer__, __LINE__) = ac::detail::Defer([&]() { fn; })
#ifndef defer
#define defer(fn) ac_defer(fn)
#else
#warning "[alt_cpp] :: 'defer' is already defined using it might end in a missbehave"
#endif

#define ac_deferc(fn) const auto AC_CONCAT(defer__, __LINE__) = ac::detail::Defer([=]() { fn; })
#ifndef deferc
#define deferc(fn) ac_deferc(fn)
#else
#warning "[alt_cpp] :: 'deferc' is already defined using it might end in a missbehave"
#endif

namespace ac::detail {
template <typename T> // <- Thanks @javiersalcedopuyo
class Defer {
public:
    Defer() = delete;
    Defer(T fn) : fn(fn) {}
    ~Defer() { fn(); }

private:
    const T fn;
};
} // namespace ac::detail

//=========================================================
//== LOGGING
//=========================================================

#ifdef ALT_CPP_INCLUDE_FMT
#undef ALT_CPP_USE_FAKE_FMT
// String Builder
#define ac_fmt(msg, ...) fmt::format(msg, __VA_ARGS__)
// Log Builder
#define __ac_log(level, msg, ...) fmt::println("[{}] | {}:{} | {}", level, __FILE__, __LINE__, ac_fmt(msg, __VA_ARGS__))
#define __ac_log_flat(msg, ...) fmt::println("{}", ac_fmt(msg, __VA_ARGS__))
#else
#if !defined(ALT_CPP_USE_FAKE_FMT)
#warning "[alt_cpp] :: Using fmt-lib will improve experience (and performance) of ac_fmt/info/err/.. methods a lot."
#endif
#include <iostream>
#include <regex>

// #ifdef _WIN32
// #include <windows.h>
// static const int ___ALT_CPP_COUT_SETUP = []() {
//     SetConsoleOutputCP(CP_UTF8);
//     return 0;
// }();
// #endif

namespace ac::detail::format {
inline std::string format(std::string msg, std::vector<std::string> const &args) {
    if (args.size() < 1) {
        return msg;
    }
#if !defined(ALT_CPP_USE_FAKE_FMT) // Append the args at the string's end
    msg += " | <== ";
    for (size_t i = 0; i < args.size() - 1; ++i) {
        msg += "{ " + args[i] + " } : ";
    }
    msg += "{ " + args[args.size() - 1] + " }";
#else                              // Replace the {} in the string
    static const std::regex pattern("\\{:?.?:?[^\\}^ ]*\\}"); // Trying to capture fmt mini-language
    auto args_it = args.begin();

    std::sregex_iterator begin(msg.begin(), msg.end(), pattern);
    std::sregex_iterator end;

    for (auto i = begin; i != end && args_it != args.end(); ++i, ++args_it) {
        msg.replace(i->position(), i->length(), *args_it);
        begin = std::sregex_iterator(msg.begin(), msg.end(), pattern); // Reset iterator after modification
    }
#endif
    return msg;
}
inline std::vector<std::string> to_stringlist() { return {}; }
template <typename T, typename... Args>
inline std::vector<std::string> to_stringlist(T &&first, Args &&...args) {
    std::ostringstream oss;
    oss << std::boolalpha << first;
    std::vector<std::string> result { oss.str() };
    std::vector<std::string> rest = to_stringlist(std::forward<Args>(args)...);
    result.insert(result.end(), rest.begin(), rest.end());
    return result;
}
} // namespace ac::detail::format

// String Builder
#define ac_fmt(msg, ...) ac::detail::format::format(msg, ac::detail::format::to_stringlist(__VA_ARGS__))

// Log Builder
#define __ac_log(level, msg, ...)                                                                                      \
    std::cout << "[" << level << "] | " << __FILE__ << ":" << __LINE__ << " | " << ac_fmt(msg, __VA_ARGS__) << "\n"
#define __ac_log_flat(msg, ...) std::cout << ac_fmt(msg, __VA_ARGS__) << "\n"
#endif

// Logging helpers
#define ac_print(msg, ...) __ac_log_flat(msg, __VA_ARGS__)
#define ac_info(msg, ...) __ac_log("INFO", msg, __VA_ARGS__)
#define ac_warn(msg, ...) __ac_log("WARN", msg, __VA_ARGS__)
#define ac_err(msg, ...) __ac_log("ERRO", msg, __VA_ARGS__)
#define ac_debug(msg, ...) __ac_log("DEBG", msg, __VA_ARGS__)

//=========================================================
//== OTHER MACROS
//=========================================================

#define ac_bind(fn) [this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define ac_bit(x) (1 << x)

#define ac_nocopy(T)                                                                                                   \
public:                                                                                                                \
    T(T const &) = delete;                                                                                             \
    T &operator=(T const &) = delete;

#define ac_nomove(T)                                                                                                   \
public:                                                                                                                \
    T(T &&) noexcept = delete;                                                                                         \
    T &operator=(T &&) noexcept = delete;

#define ac_nocopy_nomove(T) ac_nocopy(T) ac_nomove(T)

#define ac_as(T, x) static_cast<T>(x)
#ifndef as
#define as(T, x) ac_as(T, x)
#else
#warning "[alt_cpp] :: 'as' is already defined using it might end in a missbehave"
#endif


//=========================================================
//== NAMESPACE
//=========================================================

namespace ac {

//-------------------------------------
// ... Numbers Aliases
//-------------------------------------

namespace TypeAlias_Numbers {

// Bool
using b8 = bool;

// Unsigned
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = size_t;
inline constexpr u8 u8_min = std::numeric_limits<u8>::min();
inline constexpr u8 u8_max = std::numeric_limits<u8>::max();
inline constexpr u16 u16_min = std::numeric_limits<u16>::min();
inline constexpr u16 u16_max = std::numeric_limits<u16>::max();
inline constexpr u32 u32_min = std::numeric_limits<u32>::min();
inline constexpr u32 u32_max = std::numeric_limits<u32>::max();
inline constexpr u64 u64_min = std::numeric_limits<u64>::min();
inline constexpr u64 u64_max = std::numeric_limits<u64>::max();
inline constexpr usize usize_min = std::numeric_limits<usize>::min();
inline constexpr usize usize_max = std::numeric_limits<usize>::max();

// Signed
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using isize = ptrdiff_t;
inline constexpr i8 i8_min = std::numeric_limits<i8>::min();
inline constexpr i8 i8_max = std::numeric_limits<i8>::max();
inline constexpr i16 i16_min = std::numeric_limits<i16>::min();
inline constexpr i16 i16_max = std::numeric_limits<i16>::max();
inline constexpr i32 i32_min = std::numeric_limits<i32>::min();
inline constexpr i32 i32_max = std::numeric_limits<i32>::max();
inline constexpr i64 i64_min = std::numeric_limits<i64>::min();
inline constexpr i64 i64_max = std::numeric_limits<i64>::max();
inline constexpr isize isize_min = std::numeric_limits<isize>::min();
inline constexpr isize isize_max = std::numeric_limits<isize>::max();

// Floating point
using f32 = float;
using f64 = double;
inline constexpr f32 f32_min = std::numeric_limits<f32>::min();
inline constexpr f32 f32_max = std::numeric_limits<f32>::max();
inline constexpr f32 f32_epsilon = std::numeric_limits<f32>::epsilon();
inline constexpr f64 f64_min = std::numeric_limits<f64>::min();
inline constexpr f64 f64_max = std::numeric_limits<f64>::max();
inline constexpr f64 f64_epsilon = std::numeric_limits<f64>::epsilon();

} // namespace TypeAlias_Numbers
using namespace TypeAlias_Numbers;


//-------------------------------------
// ... Pointers Aliases
//-------------------------------------

namespace TypeAlias_Pointers {

// Unique pointer
template <typename T>
using Uptr = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Uptr<T> Unew(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Shared pointer
template <typename T>
using Sptr = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Sptr<T> Snew(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace TypeAlias_Pointers
using namespace TypeAlias_Pointers;


//-------------------------------------
// ... Containers Aliases
//-------------------------------------

namespace TypeAlias_Containers {

// RO : Read Only (aka: const &)
// RW : Read and Write (aka: &)

// Unordered Map
template <typename K, typename V>
using Umap = std::unordered_map<K, V>;
template <typename K, typename V>
using Umap_RO = std::unordered_map<K, V> const &;
template <typename K, typename V>
using Umap_RW = std::unordered_map<K, V> &;

// Ordered Map
template <typename K, typename V>
using Omap = std::map<K, V>;
template <typename K, typename V>
using Omap_RO = std::map<K, V> const &;
template <typename K, typename V>
using Omap_RW = std::map<K, V> const &;

// Unordered Set
template <typename T>
using Uset = std::unordered_set<T>;
template <typename T>
using Uset_RO = std::unordered_set<T> const &;
template <typename T>
using Uset_RW = std::unordered_set<T> &;

// Ordered Set
template <typename T>
using Oset = std::set<T>;
template <typename T>
using Oset_RO = std::set<T> const &;
template <typename T>
using Oset_RW = std::set<T> &;

// Dynamic Array
template <typename T>
using Vec = std::vector<T>;
template <typename T>
using Vec_RO = std::vector<T> const &;
template <typename T>
using Vec_RW = std::vector<T> &;

// Array
template <typename T, size_t S>
using Arr = std::array<T, S>;
template <typename T, size_t S>
using Arr_RO = std::array<T, S> const &;
template <typename T, size_t S>
using Arr_RW = std::array<T, S> &;

// Optional
template <typename T>
using Opt = std::optional<T>;

// String
using Str = std::string;
using Str_RO = std::string const &;
using Str_RW = std::string &;

// Function
template <typename T>
using Fn = std::function<T>;
template <typename T>
using Fn_RO = std::function<T>;

// Span
template <typename T>
using Span = std::span<T>;
template <typename T>
using Span_RO = std::span<const T>;

} // namespace TypeAlias_Containers
static inline constexpr auto None = std::nullopt;
using namespace TypeAlias_Containers;


//-------------------------------------
// ... GLM Aliases
//-------------------------------------

#ifdef ALT_CPP_INCLUDE_GLM

namespace TypeAlias_GLM {
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
} // namespace TypeAlias_GLM
using namespace TypeAlias_GLM;
#else
namespace TypeAlias_GLM {} // namespace TypeAlias_GLM
#endif


//-------------------------------------
// ... Time Consts
//-------------------------------------

inline constexpr f64 s_to_ms = 1e+3;
inline constexpr f64 s_to_us = 1e+6;
inline constexpr f64 s_to_ns = 1e+9;

inline constexpr f64 ms_to_s = 1e-3;
inline constexpr f64 ms_to_us = 1e+3;
inline constexpr f64 ms_to_ns = 1e+6;

inline constexpr f64 us_to_s = 1e-6;
inline constexpr f64 us_to_ms = 1e-3;
inline constexpr f64 us_to_ns = 1e+3;

inline constexpr f64 ns_to_s = 1e-9;
inline constexpr f64 ns_to_ms = 1e-6;
inline constexpr f64 ns_to_us = 1e-3;


//-------------------------------------
// ... Elapsed Timer
//-------------------------------------

class ElapsedTimer {
public:
    void reset();
    f64 elapsed_s() const;
    f64 elapsed_ms() const;
    f64 elapsed_us() const;
    f64 elapsed_ns() const;
    b8 is_valid() const;

private:
    i64 elapsed() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point m_ref = Clock::now();
    b8 m_valid = false;
};
using ETimer = ElapsedTimer;


//-------------------------------------
// ... String Utils
//-------------------------------------

Str str_replace(Str to_replace, Str_RO from, Str_RO to, b8 onlyFirstMatch = false);
Vec<Str> str_split(Str_RO to_split, Str_RO delimeter);


//-------------------------------------
// ... Binary Utils
//-------------------------------------

Vec<u8> bin_read(Str_RO path);
b8 bin_check_magic(Span_RO<u8> bin, Span_RO<u8> magic);


//-------------------------------------
// ... Files Utils
//-------------------------------------

Str file_read(Str_RO input_file);

b8 file_write_append(Str_RO output_file, Str_RO to_write);
b8 file_write_trunc(Str_RO output_file, Str_RO to_write);

b8 file_write_append(Str_RO output_file, const char *data, usize data_size);
b8 file_write_trunc(Str_RO output_file, const char *data, usize data_size);

b8 file_check_extension(Str_RO input_file, Str ext);


//-------------------------------------
// ... Math Utils
//-------------------------------------

f32 map(f32 value, f32 srcMin, f32 srcMax, f32 dstMin, f32 dstMax);
f32 map_100(f32 value, f32 dstMin, f32 dstMax);

b8 fuzzyEq(f32 f1, f32 f2, f32 threshold = 0.01f);

f32 clampAngle(f32 angle);

#ifdef ALT_CPP_INCLUDE_GLM
b8 fuzzyEq(Vec2 const &v1, Vec2 const &v2, f32 t = 0.01f);
b8 fuzzyEq(Vec3 const &v1, Vec3 const &v2, f32 t = 0.01f);
b8 fuzzyEq(Vec4 const &v1, Vec4 const &v2, f32 t = 0.01f);

template <typename T>
inline b8 isAligned(T const &a, T const &b, f32 margin = 0.f) {
    return abs(glm::dot(glm::normalize(a), glm::normalize(b))) >= (1.f - f32_epsilon - margin);
}
#endif

} // namespace ac


//=============================================================================
//=============================================================================
// ALT_CPP IMPLEMENTATIONs
//=============================================================================
//=============================================================================

#ifdef ALT_CPP_IMPLEMENTATION

#ifndef __ALT_CPP_IMPLEMENTATION_GUARD
#define __ALT_CPP_IMPLEMENTATION_GUARD

#include <fstream>

namespace ac {

//-------------------------------------
// ... Elapsed Timer
//-------------------------------------

void ElapsedTimer::reset() {
    m_valid = true;
    m_ref = Clock::now();
}
f64 ElapsedTimer::elapsed_s() const { return as(f64, elapsed()) * ns_to_s; }
f64 ElapsedTimer::elapsed_ms() const { return as(f64, elapsed()) * ns_to_ms; }
f64 ElapsedTimer::elapsed_us() const { return as(f64, elapsed()) * ns_to_us; }
f64 ElapsedTimer::elapsed_ns() const { return as(f64, elapsed()); }
b8 ElapsedTimer::is_valid() const { return m_valid; }
i64 ElapsedTimer::elapsed() const {
    auto const now = Clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_ref).count();
}


//-------------------------------------
// ... String Utils
//-------------------------------------

Str str_replace(Str to_replace, Str_RO from, Str_RO to, b8 only_first_match) {
    usize pos = 0;
    while ((pos = to_replace.find(from)) < to_replace.size()) {
        to_replace.replace(pos, from.length(), to);
        if (only_first_match) {
            break;
        }
    }
    return to_replace;
}

Vec<Str> str_split(Str_RO to_split, Str_RO delimeter) {
    Str token;
    Vec<Str> splitted;
    usize ini = 0;
    usize end = 0;

    // Split and store the string body
    while ((end = to_split.find(delimeter, ini)) < to_split.size()) {
        token = to_split.substr(ini, end - ini);
        ini = end + delimeter.size();
        splitted.push_back(token);
    }

    // Store the string tail
    if (ini < to_split.size()) {
        token = to_split.substr(ini);
        splitted.push_back(token);
    }

    return splitted;
}


//-------------------------------------
// ... Binary Utils
//-------------------------------------

Vec<u8> bin_read(Str_RO path) {
    std::ifstream file { path, std::ios::binary };
    auto fileBegin = std::istreambuf_iterator<char>(file);
    auto fileEnd = std::istreambuf_iterator<char>();
    return { fileBegin, fileEnd };
}

b8 bin_check_magic(Span_RO<u8> bin, Span_RO<u8> magic) {
    // Validation
    if (magic.empty() || bin.size() < magic.size()) {
        return false;
    }
    // Iteration
    b8 match = true;
    for (usize i = 0; i < magic.size(); ++i) {
        match &= (bin[i] == magic[i]);
    }
    // Result
    return match;
}


//-------------------------------------
// ...Files Utils
//-------------------------------------

Str file_read(Str_RO input_file) {
    std::ifstream file(input_file, std::ios::ate | std::ios::binary);
    ac_defer(file.close());

    if (!file.is_open()) {
        return "";
        ac_err("Issues opening file [r]: {}", input_file);
    }

    Str content;
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());

    return content;
}

b8 file_write(Str_RO output_file, char const *data, usize data_size, std::ios_base::openmode mode) {
    if (!data || data_size < 1) {
        return false;
        ac_err("[file_write] Invalid data: {}", output_file);
    }

    std::ofstream file(output_file, std::ios::out | std::ios::binary | mode);
    ac_defer(file.close());

    if (!file.is_open()) {
        return false;
        ac_err("[file_write] Opening file: {}", output_file);
    }

    file.write(data, data_size);

    return true;
}
b8 file_write_append(Str_RO output_file, Str_RO to_write) {
    return file_write(output_file, to_write.data(), to_write.size(), std::ios::app);
}
b8 file_write_trunc(Str_RO output_file, Str_RO to_write) {
    return file_write(output_file, to_write.data(), to_write.size(), std::ios::trunc);
}
b8 file_write_append(Str_RO output_file, const char *data, usize data_size) {
    return file_write(output_file, data, data_size, std::ios::app);
}
b8 file_write_trunc(Str_RO output_file, const char *data, usize data_size) {
    return file_write(output_file, data, data_size, std::ios::trunc);
}

b8 file_check_extension(Str_RO input_file, Str ext) {
    auto to_check = input_file.substr(input_file.find_last_of('.') + 1);
    std::transform(to_check.begin(), to_check.end(), to_check.begin(), ::tolower);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return to_check == ext;
}


//-------------------------------------
// ... Math Utils
//-------------------------------------

f32 map(f32 value, f32 srcMin, f32 srcMax, f32 dstMin, f32 dstMax) {
    return dstMin + (dstMax - dstMin) * (value - srcMin) / (srcMax - srcMin);
}
f32 map_100(f32 value, f32 dstMin, f32 dstMax) { return map(value, 0, 100, dstMin, dstMax); }
b8 fuzzyEq(f32 f1, f32 f2, f32 threshold) {
    auto const diff = abs(f1 - f2);
    auto const isEq = diff <= threshold;
    return isEq;
}
f32 clampAngle(f32 angle) {
    auto const turns = floorf(angle / 360.f);
    return angle - 360.f * turns;
}

#ifdef ALT_CPP_INCLUDE_GLM
b8 fuzzyEq(Vec2 const &v1, Vec2 const &v2, f32 t) { return fuzzyEq(v1.x, v2.x, t) && fuzzyEq(v1.y, v2.y, t); }
b8 fuzzyEq(Vec3 const &v1, Vec3 const &v2, f32 t) {
    return fuzzyEq(v1.x, v2.x, t) && fuzzyEq(v1.y, v2.y, t) && fuzzyEq(v1.z, v2.z, t);
}
b8 fuzzyEq(Vec4 const &v1, Vec4 const &v2, f32 t) {
    return fuzzyEq(v1.x, v2.x, t) && fuzzyEq(v1.y, v2.y, t) && fuzzyEq(v1.z, v2.z, t) && fuzzyEq(v1.w, v2.w, t);
}
#endif


//-------------------------------------
// ...
//-------------------------------------

} // namespace ac

#endif // __ALT_CPP_IMPLEMENTATION_GUARD

#endif // ALT_CPP_IMPLEMENTATION
