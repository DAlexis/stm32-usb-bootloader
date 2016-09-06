/*
 * cardreader.h
 *
 *  Created on: 15 дек. 2015 г.
 *      Author: alexey
 */

#ifndef INC_CARDREADER_H_
#define INC_CARDREADER_H_

#include <stdint.h>

void initCardreader();
void deinitCardreader();
int isCardreaderActive();

/**
 * Enable cardreader for duration and stop it if no USB connection occured.
 * Otherwise infinite cardreader mode.
 */
void temporaryEnableCardreader(uint32_t duration);

#endif /* INC_CARDREADER_H_ */
