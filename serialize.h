#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <stdbool.h>
#include <stdint.h>
#include "error.h"

/*
 * This macro defines how long (in bytes) a string must be in order to
 * hold a serialized system_state. It includes the null terminator, so
 * a serialized system state should have SERIALIZED_OUTPUT_LEN - 1 ascii
 * characters in it
 */
#define SERIALIZED_OUTPUT_LEN 3
/*
 * Length of a state command. A state command is a block of characters
 * that can be sent over the radio. It's mostly the serialized state output
 * with a couple of additional bytes for STATE_COMMAND header and for a
 * CRC or hamming code
 */
#define STATE_COMMAND_LEN 5
/*
 * This character indicates the beginning of a state command.
 */
#define STATE_COMMAND_HEADER '{'

/*
 * This type contains all of the information that needs to be shared between
 * the operator on the ground and the CAN system in the rocket.
 */
typedef struct {
    uint8_t num_boards_connected;
    bool injector_valve_open;
    bool vent_valve_open;
    bool running_self_test;
    bool any_errors_detected;
} system_state;

/*
 * This function converts a binary value from 0 to 63 inclusive into a
 * printable charcter using a modified version of Base64. The + character is
 * not used, and is replaced by the & character, due to the XBEE interpreting
 * the + character as a special character.
 */
char binary_to_base64(uint8_t binary);

/*
 * This function converts a value from a modified version of Base64 into a raw
 * binary value. See the description of the function binary_to_base64() for a
 * description of the modified Base64 encoding and the reason for its use.
 */
uint8_t base64_to_binary(char base64);

/*
 * This function takes a system_state and serializes it into ASCII text that
 * can be sent over the radio. It will return true if it was able to
 * successfully serialize the state, false otherwise. Examples of how it can
 * return false is passing it a NULL pointer as either argument
 */
bool serialize_state(const system_state *state, char *str);

/*
 * This function takes a string which was generated by the serialize_state
 * function and converts it back into a system_state. Returns true if it
 * was successfully able to deserialize a state, false otherwise
 */
bool deserialize_state(system_state *state, const char *str);

/*
 *
 */
bool serialize_error(const error_t *err, char *str);

/*
 *
 */
bool deserialize_error(error_t *err, const char *str);

/*
 * Returns true if the two system states passed to it are equal (returns
 * false if either of them are NULL). Note that in C you're not just allowed
 * to do (*s == *p), you have to individually compare each element in the
 * struct, due to data representation reasons
 */
bool compare_system_states(const system_state *s, const system_state *p);

/*
 * This function creates a state command that can be sent over the radio
 * byte by byte. Returns false if it couldn't do so, for some reason
 */
bool create_state_command(char *cmd, const system_state *state);

/*
 * This function computes the checksum of a NULL-terminated message using a
 * modified version of the Luhn algorithm. The checksum is equal to the sum of
 * the odd-placed digits plus three times the sum of the even-placed digits,
 * modulo 64. The function returns the Base-64 encoding of the checksum.
 */
char checksum(char *cmd);

#endif
