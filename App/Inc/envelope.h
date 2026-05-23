/*
 * envelope.h
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#ifndef INC_ENVELOPE_H_
#define INC_ENVELOPE_H_

#include "main.h"

typedef struct Envelope Envelope_t;

Envelope_t* createEnvelope(void);
void envelopeRegister(Envelope_t* envelope);
void initEnvelope(Envelope_t* envelope);
float processSampleEnvelope(Envelope_t* envelope);

void setEnvelopeAttack(Envelope_t* envelope,  float attackPotentiometer);
float getEnvelopeAttack(Envelope_t* envelope);
void setEnvelopeGate(Envelope_t* envelope, uint8_t wantedGateSate);

#endif /* INC_ENVELOPE_H_ */
