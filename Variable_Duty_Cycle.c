/*

	C51 Variable Duty Cycle Frequency Generator
	GitHub: https://github.com/SubNader/C51FrequencyGenerator
	Credits: Nader Alsharkawy (3001) and Mohamed Amgad (3009) | Group 36

*/

#include <at89c51xd2.h> 
#include <stdio.h>

// Input port definition
#define input_port P1

// Ouput pins definition
sbit output_bit = P2 ^ 0;
sbit output_switch = P2 ^ 7;
sbit error = P2 ^ 6;

// Delay function prototype
void delay(unsigned char high, unsigned char low);

// Variables definition - chars for 8 bits / floats for numerical values
unsigned char input, updated_input, input_high, input_low,
positive_high, positive_low,
negative_high, negative_low;
float internal_frequency, duty_cycle, total_cycles_count,
positive_cycles_count, positive_cycles_start_value,
negative_cycles_count, negative_cycles_start_value,
max_cycles = 65536;

void main(void) {
  // Initialize input port
  input_port = 0xFF;

  // Enable timer 0 in mode 1 - 0000 0001
  TMOD = 0x01;

  // Initialize pins
  output_bit = 0;
  output_switch = 1;
  error = 1;

  // Compute the internal MC frequency - assuming a 22.1184MHz crystal
  internal_frequency = 22118400 / 12.0;

  // Compute the total number of cycles (full period)
  total_cycles_count = (1 / 500.0) / (1 / internal_frequency);

  while (1) {
    //Fetch and handle low and high nibbles from input port
    input_high = (input_port & 0xF0) >> 4;
    input_low = input_port & 0x0F;

    if (output_switch == 0) {

      // Validate and handle invalid input
      if (!(input_high > 0x09 || input_low > 0x09)) {
        error = 1;
        // Fetch and handle low and high nibbles from input port
        input_high = (input_port & 0xF0) >> 4;
        input_low = input_port & 0x0F;

        // Compute duty cycle
        duty_cycle = input_high * 10 + input_low;

        // Compute the positive and negative cycles and the corresponding timer start values based on the duty cycle
        positive_cycles_count = (int)((duty_cycle / 100.0) * total_cycles_count);
        negative_cycles_count = total_cycles_count - positive_cycles_count;
        positive_cycles_start_value = max_cycles - positive_cycles_count;
        negative_cycles_start_value = max_cycles - negative_cycles_count;

        // Compute low and high byte values from computed start values
        positive_low = (int) positive_cycles_start_value & 0x00FF;
        positive_high = ((int) positive_cycles_start_value & 0xFF00) >> 8;
        negative_low = (int) negative_cycles_start_value & 0x00FF;
        negative_high = ((int) negative_cycles_start_value & 0xFF00) >> 8;

        // Handle runtime input update
        input = updated_input = input_port;

        // Ouput loop
        while (input == updated_input && output_switch == 0) {

          // Positive portion
          output_bit = ~output_bit;
          delay(positive_high, positive_low);

          // Negative portion
          output_bit = ~output_bit;
          delay(negative_high, negative_low);

          // Fetch input port state to check if altered
          updated_input = input_port;
        }
      } else {
        // Stop output
        output_bit = 0;

        // Enable error indication
        error = 0;
      }
    } else {
      // Disable error indication
      error = 1;
    }
  }
}

// Delay function
void delay(unsigned char high, unsigned char low) {

  // Set timer's start value
  TH0 = high;
  TL0 = low;

  // Start timer
  TR0 = 1;

  // Loop till rollover
  while (TF0 == 0);

  // Clear timer run (enable) and flag
  TR0 = 0;
  TF0 = 0;
}