#ifndef LIB_LOG_H
#define LIB_LOG_H

/**
 * @brief Macro to be used in a formatted string to a pass float number to the log.
 *
 * Macro should be used in formatted string instead of the %f specifier together with
 * @ref NRF_LOG_FLOAT macro.
 * Example: LOG("My float number" LOG_FLOAT_MARKER "\r\n", LOG_FLOAT(f))
 */
#define LOG_FLOAT_MARKER "%s%d.%02d"

/**
 * @brief Macro for dissecting a float number into two numbers (integer and residuum).
 */
#define LOG_FLOAT(val) (uint32_t)(((val) < 0 && (val) > -1.0) ? "-" : ""),   \
                           (int32_t)(val),                                       \
                           (int32_t)((((val) > 0) ? (val) - (int32_t)(val)       \
                                                : (int32_t)(val) - (val))*100)


uint32_t log_hexdump(const uint8_t *p_data, uint16_t len);

#endif //__LIB_LOG_H__
