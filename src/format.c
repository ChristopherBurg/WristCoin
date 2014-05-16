#include "format.h"

/* In order to allocate the appropriate amount of memory for a format string the
 * number of digits in the number must be known. This function continues to
 * divide the passed in number by 10 until it has determined the number of
 * digits it contains.
 *
 * int64_t num - The number to count the digits of.
 */
static int get_digits(int64_t num) {
  int digits = 0;
  /* We're counting digits so whether the number is positive or negative is
   * irrelevant but the math only works with a positive number.
   */
  int64_t temp = abs(num);

  digits++;
  while (temp >= 10) {
    temp /= 10;
    digits++;
  }

  return digits;
}

/* Takes an integer value and produces a string formatted as currency. It is
 * assumed that the last two digits in the integer are decimal values. So
 * passing in something like '12345' will produce the string '$ 123.45'.
 *
 * int64_t price - The integer value to format.
 */
char * create_format_dollars(int64_t price) {
  char *dollars = NULL;
  int64_t characteristic = 0;
  int64_t mantissa = 0;
  int digits = 0;

  if (price < 100) {
    /* The minimum value that can be printed by this function is '$ 0.00'. If
     * the price is less than 100 then three digits should be reserved.
     */
    digits = 3;
  } else {
    digits = get_digits(price);
  }

  /* If the price is negative we need an additional character to represent the
   * '-'.
   */
  if (price < 0) {
    digits++;
  }

  /* We need four additional characters for the '$ ', decimal point, and null
   * terminator.
   */
  dollars = (char *) malloc(sizeof(char) * (digits + 4));

  characteristic = price / 100;
  mantissa = abs(price - (characteristic * 100));

  snprintf(dollars, digits + 4, "$ %lld.%02lld", characteristic, mantissa);

  return dollars;
}

char * create_format_volume(int64_t vol) {
  char *dollars = NULL;
  int64_t characteristic = 0;
  int64_t mantissa = 0;
  int digits = 0;

  /* Bitcoin values can be divide into either decimal places. That being the
   * case the shortest number of digits should be 1 to precent the decimal
   * place and either to succeed it.
   */
  if (vol < (10 ^ 9)) {
    digits = 9;
  } else {
    digits = get_digits(vol);
  }

  /* We need two additional characters for the decimal point and null
   * terminator.
   */
  dollars = (char *) malloc(sizeof(char) * (digits + 2));

  characteristic = vol / 100000000;
  mantissa = vol - (characteristic * 100000000);

  snprintf(dollars, digits + 2, "%lld.%08lld", characteristic, mantissa);

  return dollars;
}

/* Takes a formatted string and destroys it, freeing its memory. This function
 * isn't format specific, it could destroy allocated chunk of memory, but having
 * this function satisfies my OCD tendencies.
 */
void destroy_format(char *format) {
  if (format != NULL) {
    free(format);
    format = NULL;
  }
}
