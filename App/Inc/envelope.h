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

typedef enum{
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE,
	IDLE
} ADSRstate_t;

Envelope_t* createEnvelope(void);
void initEnvelope(Envelope_t* envelope);
float processSampleEnvelope(Envelope_t* envelope);

float getEnvelopeAttackTime(Envelope_t* envelope);
float getEnvelopeReleaseTime(Envelope_t* envelope);

void setEnvelopeCurrentLevel(Envelope_t* envelope, float currentVolume);
void setEnvelopeGate(Envelope_t* envelope, uint8_t wantedGateSate);
void setEnvelopeAttackTime(Envelope_t* envelope, float attackTime);
void setEnvelopeReleaseTime(Envelope_t* envelope, float releaseTime);
void setEnvelopeState(Envelope_t* envelope, ADSRstate_t adsrState);


#endif /* INC_ENVELOPE_H_ */
