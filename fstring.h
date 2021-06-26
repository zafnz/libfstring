/** @file fstring.h
 *  @brief A C version of python's fstring
 *
 *  @copyright Copyright 2021 - Nick Clifford <nick@crypto.geek.nz>
 * 
 *  @author - Nick Clifford (nick@crypto.geek.nz)
 */
#ifndef include_fstring_h
#define include_fstring_h

#include <sys/types.h>

/**
 * @brief The callback type for dynamic values. 
 */
typedef const char *(*fstring_callback_t)(void *data, const char *name);

/**
 * @brief Values to pass to fstring's values list
 * 
 */
typedef struct {
    const char *name;
    const char *value;
    fstring_callback_t callback;
    void *callback_data;
} fstring_value;


/**
 * @brief Formatted string using supplied variables (similar to Python's fstrings)
 * 
 * @details Fills buffer with an output as described below.
 * 
 * The format string is a character string that is copied to the buffer. Where a
 * value directive occurs, which is a string surrounded by curly braces, the
 * provided values list is looked up for the corresponding value, and that value
 * is substituded into the string. The value can be any length, longer or shorter
 * than the name without issue. 
 * 
 * @param[out] buffer     The buffer to store the output in, based on the contents of
 *                   format and the values. If there is an error condition, the
 *                   contents of buffer is unspecified and may not be NULL 
 *                   terminated.
 * 
 * @param[in] buffer_len The maximum length of the buffer, the string will be NULL 
 *                   terminated when writing to the buffer, buffer_len needs to
 *                   be large enough to include the NULL character.
 * 
 * @param[in] format     The string to parse to produce the output. The string can
 *                   contain variable names enclosed with braces, and when variables
 *                   are found they are looked up in the values list
 * 
 * @param[in] values     The values of the variables are in this list. The list is an
 *                   array of fstring_value structures, where the last structure's
 *                   name property must be NULL.
 * 
 * @return           Returns the formatted string.
 * 
 * ## Examples
 * 
 * Inline passing
 * @code
 * fstring(buffer, sizeof(buffer), "This is a {variable} which is {what}", (fstring_value[]) {
 *          {.name="variable", .value="thing"},
 *          {.name="what", .value="cool"}
 *          {.name=NULL}
 * });
 * // Returns "This is a thing which is cool"
 * @endcode
 * 
 * More expanded
 * @code
 * fstring_value values[] = {
 *      { .name="variable", .value="thing" },
 *      { .name="what", .value="cool" }
 * };
 * fstring(buffer, sizeof(buffer), "This is a {variable} which is {what}", values);
 * // Returns "this is a thing which is cool"
 * @endcode
 */
extern int fstring(char *buffer, size_t buffer_len, const char *format, fstring_value *values);

#endif