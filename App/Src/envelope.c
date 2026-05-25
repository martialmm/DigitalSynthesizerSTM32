/*
 * envelope.c
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#include <stddef.h>
#include "envelope.h"

struct Envelope{
	float attack;
	float attackTime;
	float decay;
	float sustain;
	float release;
	float releaseTime;
	uint8_t gate;
};

Envelope_t* createEnvelope(void) {
    static Envelope_t instance = {0};
    return &instance;
}

void initEnvelope(Envelope_t* envelope){
	envelope->attack = 0.0f;
	envelope->attackTime = 0.0f;
	envelope->decay = 0.0f;
	envelope->sustain = 0.0f;
	envelope->release = 0.0f;
	envelope->releaseTime = 0.0f;
	envelope->gate = 0;
}

float processSampleEnvelope(Envelope_t* envelope){
	float attackRate = 1.0f / (SAMPLE_RATE * envelope->attackTime);
	float releaseRate = 1.0f / (SAMPLE_RATE * envelope->releaseTime);

	if(envelope->gate){
		envelope->attack += attackRate;
		if(envelope->attack > 1.0f) envelope->attack = 1.0f;
	}
	else{
		envelope->attack = 0.0f;
	}

	if(envelope->gate == 0 && envelope->attack == 1.0f){
		envelope->attack -= releaseRate;
	}

	return envelope->attack;
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
	envelope->attackTime = attackTime;
}

void setEnvelopeReleaseTime(Envelope_t* envelope, float releaseTime){
	envelope->releaseTime = releaseTime;
}
