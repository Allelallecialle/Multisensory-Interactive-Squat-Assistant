#include "0_config.h"

#ifndef UTILS_H
#define UTILS_H
/**
 * @brief function for handling received messages
 * 
 */
void handle_received_message(char *received_message);

/**
 * @brief function for receiving messages
 * 
 */
void receive_message(boolean new_message_received, char* received_message);

/**
 * @brief function for looping analog and digital inputs
 * 
 */
void analog_digital_loop();

#endif



