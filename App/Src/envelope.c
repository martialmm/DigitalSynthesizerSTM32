/*
 * envelope.c
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#include "envelope.h"
#include "main.h"

static Envelope_t* envelope1 = NULL;

struct Envelope{
	float attack;
	float attackRate;
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
	float attackRate = 0.00001f;
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
