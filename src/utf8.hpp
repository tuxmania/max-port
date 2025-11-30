/* Copyright (c) 2025 M.A.X. Port Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UTF8_HPP
#define UTF8_HPP

#include <string>

/**
 * @brief Converts a UTF-8 string to its lowercase equivalent.
 *
 * This function takes a UTF-8 encoded string as input and returns a new string
 * with all characters converted to their lowercase forms, according to Unicode
 * case mapping rules.
 *
 * @param input The UTF-8 encoded string to be converted.
 * @return A new std::string containing the lowercase version of the input string.
 */
std::string utf8_tolower_str(const std::string& input);

/**
 * @brief Converts a UTF-8 string to its uppercase equivalent.
 *
 * This function takes a UTF-8 encoded string as input and returns a new string
 * with all characters converted to their uppercase forms, according to Unicode
 * case mapping rules.
 *
 * @param input The UTF-8 encoded string to be converted.
 * @return A new std::string containing the uppercase version of the input string.
 */
std::string utf8_toupper_str(const std::string& input);

/**
 * @brief Calculates the number of characters in a UTF-8 encoded C-style string.
 *
 * This function correctly counts the number of Unicode codepoints (characters)
 * in a null-terminated UTF-8 string, as opposed to `strlen`, which counts bytes.
 *
 * @param str A pointer to a null-terminated, UTF-8 encoded C-style string.
 * @return The number of Unicode characters in the string.
 */
size_t utf8_strlen(const char* str);

/**
 * @brief Finds the byte offset of a character at a given codepoint index in a UTF-8 string.
 *
 * This is useful for substrings and character-level operations on UTF-8 strings,
 * allowing correct indexing by character rather than by byte.
 *
 * @param str A pointer to a null-terminated, UTF-8 encoded C-style string.
 * @param codepoint_index The character index (not byte index) to find.
 * @return The byte offset from the start of the string to the beginning of the specified character.
 */
size_t utf8_byte_offset(const char* str, size_t codepoint_index);

/**
 * @brief Gets the byte offset of the character immediately preceding the given byte offset.
 *
 * This function is essential for reverse iteration or finding character boundaries
 * in a UTF-8 string from a given byte position.
 *
 * @param str A pointer to a null-terminated, UTF-8 encoded C-style string.
 * @param byte_offset The starting byte offset from which to find the previous character.
 * @return The byte offset of the start of the previous character.
 */
size_t utf8_prev_char_offset(const char* str, size_t byte_offset);

/**
 * @brief Gets the byte offset of the character immediately following the given byte offset.
 *
 * This function is useful for forward iteration and locating the start of the next
 * character in a UTF-8 string from a given byte position.
 *
 * @param str A pointer to a null-terminated, UTF-8 encoded C-style string.
 * @param byte_offset The starting byte offset from which to find the next character.
 * @return The byte offset of the start of the next character.
 */
size_t utf8_next_char_offset(const char* str, size_t byte_offset);

/**
 * @brief Decodes a UTF-8 string into an 8-bit extended ASCII string.
 *
 * This function converts a UTF-8 string into a legacy 8-bit character encoding,
 * which is necessary for rendering text with the game's original engine. It
 * maps Unicode codepoints to their corresponding single-byte representations.
 * Any characters that cannot be represented in the 8-bit set are replaced with '?'.
 *
 * @param input The UTF-8 string to be decoded.
 * @return A std::string containing the 8-bit extended ASCII representation.
 */
std::string utf8_decode(const std::string& input);

#endif /* UTF8_HPP */
