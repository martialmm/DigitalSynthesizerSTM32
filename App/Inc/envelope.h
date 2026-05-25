/*
 * envelope.h
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#ifndef INC_ENVELOPE_H_
#define INC_ENVELOPE_H_

#include <stdint.h>

#define SAMPLE_RATE 96000

typedef struct Envelope Envelope_t;

Envelope_t* createEnvelope(void);
void initEnvelope(Envelope_t* envelope);
float processSampleEnvelope(Envelope_t* envelope);

float getEnvelopeAttack(Envelope_t* envelope);
float getEnvelopeAttackTime(Envelope_t* envelope);
float getEnvelopeReleaseTime(Envelope_t* envelope);

void setEnvelopeAttack(Envelope_t* envelope,  float attackPotentiometer);
void setEnvelopeGate(Envelope_t* envelope, uint8_t wantedGateSate);
void setEnvelopeAttackTime(Envelope_t* envelope, float attackTime);
void setEnvelopeReleaseTime(Envelope_t* envelope, float releaseTime);


#endif /* INC_ENVELOPE_H_ */
