/*
 * envelope.c
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#include <stddef.h>
#include "envelope.h"

typedef enum{
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE,
	IDLE
} ADSRstate_t;

struct Envelope{
	ADSRstate_t adsrState;
	float attack;
	float attackTime;
	float attackRate;
	float decay;
	float sustain;
	float release;
	float releaseTime;
	float releaseRate;
	uint8_t gate;
};


Envelope_t* createEnvelope(void) {
    static Envelope_t instance = {0};
    return &instance;
}

void initEnvelope(Envelope_t* envelope){
	envelope->adsrState = IDLE;
	envelope->attack = 0.0f;
	envelope->attackTime = 0.0f;
	envelope->attackRate = 0.0f;
	envelope->decay = 0.0f;
	envelope->sustain = 0.0f;
	envelope->release = 0.0f;
	envelope->releaseTime = 0.0f;
	envelope->releaseRate = 0.0f;
	envelope->gate = 0;
}

float processSampleEnvelope(Envelope_t* envelope){
	float output = 0.0f;

	switch(envelope->adsrState){

	case IDLE:
		if(envelope->gate) envelope->adsrState = ATTACK;
		break;

	case ATTACK:
		envelope->attack += envelope->attackRate;

		if(envelope->attack >= 1.0f) envelope->attack = 1.0f;
		output = envelope->attack;
		if(output == 1.0f && !envelope->gate) envelope->adsrState = RELEASE;
		break;

	case RELEASE:
		envelope->release -= envelope->releaseRate;
		if(output <= 0.0f){
			envelope->release = 0.0f;
		}
		output = envelope->release;
		if(envelope->gate) envelope->adsrState = ATTACK;
		else if(output == 0.0f && !envelope->gate) envelope->adsrState = IDLE;
		break;
	}
	return output;
}


float getEnvelopeAttack(Envelope_t* envelope){
	return envelope->attack;
}

float getEnvelopeAttackTime(Envelope_t* envelope){
	return envelope->attackTime;
}

float getEnvelopeReleaseTime(Envelope_t* envelope){
	return envelope->releaseTime;
}

void setEnvelopeAttack(Envelope_t* envelope, float attack){
	envelope->attack = attack;
}

void setEnvelopeGate(Envelope_t* envelope, uint8_t wantedGateSate){
	envelope->gate = wantedGateSate;
}

void setEnvelopeAttackTime(Envelope_t* envelope, float attackTime){
	if(attackTime < 0.001f) attackTime = 0.001f;
	envelope->attackRate = 1.0f / (SAMPLE_RATE * attackTime);
	envelope->attackTime = attackTime;
}

void setEnvelopeReleaseTime(Envelope_t* envelope, float releaseTime){
	envelope->releaseRate = 1.0f / (SAMPLE_RATE * releaseTime);
	envelope->releaseTime = releaseTime;
}
