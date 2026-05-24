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
	uint8_t gate;
};

Envelope_t* createEnvelope(void) {
    static Envelope_t instance = {0};
    return &instance;
}

void envelopeRegister(Envelope_t* envelope){
	envelope1 = envelope;
}

void initEnvelope(Envelope_t* envelope){
	envelopeRegister(envelope);
	envelope->attack = 0;
	envelope->attackRate = 0;
	envelope->decay = 0;
	envelope->sustain = 0;
	envelope->release = 0;
	envelope->gate = 0;
}

float processSampleEnvelope(Envelope_t* envelope){
	float attackTime = 1.0f; // seconds
	float attackRate = 1.0f / (SAMPLE_RATE * attackTime);
	if(envelope->gate){
		envelope->attack += attackRate;
		if(envelope->attack > 1.0f) envelope->attack = 1.0f;
	}
	else{
		envelope->attack = 0.0f;
	}
	return envelope->attack;
}


float getEnvelopeAttack(Envelope_t* envelope){
	return envelope->attack;
}

void setEnvelopeAttack(Envelope_t* envelope, float attackPotentiometer){

}

void setEnvelopeGate(Envelope_t* envelope, uint8_t wantedGateSate){
	envelope->gate = wantedGateSate;
}

void setEnvelopeAttackTime(Envelope_t* envelope, float attackTime){
	envelope->attackTime = attackTime;
}
