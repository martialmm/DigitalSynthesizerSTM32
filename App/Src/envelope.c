/*
 * envelope.c
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#include <stddef.h>
#include "envelope.h"

struct Envelope{
	ADSRstate_t adsrState;
	float currentLevel;
	float attackTime;
	float attackRate;
	float decay;
	float sustain;
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
	envelope->currentLevel = 0.0f;
	envelope->attackTime = 0.0f;
	envelope->attackRate = 0.0f;
	envelope->decay = 0.0f;
	envelope->sustain = 0.0f;
	envelope->releaseTime = 0.0f;
	envelope->releaseRate = 0.0f;
	envelope->gate = 0;
}

float processSampleEnvelope(Envelope_t* envelope){
	switch(envelope->adsrState){

	case IDLE:
		if(envelope->gate) envelope->adsrState = ATTACK;
		break;

	case ATTACK:
		envelope->currentLevel+= envelope->attackRate;

		if(envelope->currentLevel >= 1.0f) envelope->currentLevel = 1.0f;
		if(envelope->currentLevel == 1.0f && !envelope->gate) envelope->adsrState = RELEASE;
		break;

	case RELEASE:
		envelope->currentLevel -= envelope->releaseRate;
		if(envelope->currentLevel <= 0.0f){
			envelope->currentLevel = 0.0f;
		}
		if(envelope->gate) envelope->adsrState = ATTACK;
		else if(envelope->currentLevel == 0.0f && !envelope->gate) envelope->adsrState = IDLE;
		break;
	}
	return envelope->currentLevel;
}

float getEnvelopeAttackTime(Envelope_t* envelope){
	return envelope->attackTime;
}

float getEnvelopeReleaseTime(Envelope_t* envelope){
	return envelope->releaseTime;
}

void setEnvelopeCurrentLevel(Envelope_t* envelope, float currentLevel){
	envelope->currentLevel = currentLevel;
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
	if(releaseTime < 0.001f) releaseTime = 0.001f;
	envelope->releaseRate = 1.0f / (SAMPLE_RATE * releaseTime);
	envelope->releaseTime = releaseTime;
}

void setEnvelopeState(Envelope_t* envelope, ADSRstate_t adsrState){
	envelope->adsrState = adsrState;
}
