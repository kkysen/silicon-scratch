#pragma once

#include <iostream>
#include <string>
#include <vector>

/**
 * \brief Deserializes the basic input type. Deserializes floating number from its binary representation.
 *
 * \tparam  R  Type of the value to be deserialized.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized value.
 *
 * \return  Count of read elements.
 */
template <typename R, typename E, typename Traits>
std::ios::pos_type deserialize(std::basic_istream<E, Traits>& stream, R& out) {
    stream.read(reinterpret_cast<E*>(&out), sizeof(R));
    return stream.gcount();
}

/**
 * \brief Deserializes the string from the stream.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized string.
 * \param   length    The expected length of the string. The value represents the amount of ELEM_TYPE to be read.
 *                    
 * \return  Count of read elements.
 */
template <typename E, typename Traits, typename Allocator, template <typename, typename, typename> class String>
std::ios::pos_type deserialize(std::basic_istream<E, Traits>& stream,
        String<E, Traits, Allocator>& out, size_t length) {
    if (length > 0) {
        out.resize(length, E(0));
        stream.read(reinterpret_cast<E*>(out.data()), length);
        return stream.gcount();
    }
    return 0;
}

/**
 * \brief Deserializes the vector from the stream.
 * \param   stream    The stream to be deserialized from.
 * \param   [out] out The deserialized string.
 * \param   length    The expected amount of elements in the serialized vector.
 *                    
 * \return  Count of read elements.
 */
template <typename E, typename Traits, typename VectorElement, typename Allocator, template <typename, typename> class Vector>
std::ios::pos_type
deserialize(std::basic_istream<E, Traits>& stream, Vector<VectorElement, Allocator>& out, size_t length) {
    static_assert(sizeof(E) == sizeof(VectorElement), "sizes must match");
    
    if (length > 0) {
        out.resize(length, VectorElement());
        stream.read(reinterpret_cast<E*>(out.data()), length);
        return stream.gcount();
    }
    
    return 0;
}

/**
 * \brief Serializes the basic input type. Serializes floating number into its binary representation.
 *
 * \tparam  T      Type of the value to be serialized.
 * \param   stream    The stream to be serialized into.
 * \param   value     The value to be serialized.
 */
template <typename T, typename E, typename Traits>
void serialize(std::basic_ostream<E, Traits>& stream, const T& value) {
    stream.write(reinterpret_cast<const E*>(&value), sizeof(T) / sizeof(E));
}

/**
 * \brief Serializes the string.
 * \param   stream    The stream to be serialized into.
 * \param   value     The string to be serialized.
 */
template <typename E, typename Traits, typename Allocator, template <typename, typename, typename> class String>
void serialize(std::basic_ostream<E, Traits>& stream, const String<E, Traits, Allocator>& value) {
    stream.write(reinterpret_cast<const E*>(value.data()), value.length());
}

/**
 * \brief Serializes the vector.
 *
 * \param   stream    The stream to be serialized into.
 * \param   value     The vector to be serialized.
 */
template <typename E, typename Traits, typename VectorElement, typename Allocator, template <typename, typename> class Vector>
void serialize(std::basic_ostream<E, Traits>& stream,
               const Vector<VectorElement, Allocator>& value) {
    static_assert(sizeof(E) == sizeof(VectorElement), "sizes must match");
    stream.write(reinterpret_cast<const E*>(value.data()), value.size());
}
