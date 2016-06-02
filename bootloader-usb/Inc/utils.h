/*
 * utils.h
 *
 *  Created on: 10 дек. 2015 г.
 *      Author: alexey
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#include <stdint.h>

#define assert_fatal(expr, what, code) ((expr) ? (void)0 : fatal_error(what, code, (uint8_t *)__FILE__, __LINE__))

void delay();
uint32_t HashLy(uint32_t hash, const uint8_t * buf, uint32_t size);
void resetAllPins();

void assert_failed(uint8_t* file, uint32_t line);
void fatal_error(char* what, int errCode, uint8_t* file, uint32_t line);

#endif /* INCLUDE_UTILS_H_ */
