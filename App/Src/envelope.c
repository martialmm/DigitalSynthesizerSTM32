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
	float decay;
	float sustain;
	float release;
	uint8_t gate;
};

Envelope_t* createEnvelope(void) {
    static Envelope_t instance = {0};
    return &instance;
}

void envelopeRegister(Envelope_t* envelope) {
	envelope1 = envelope;
}

void initEnvelope(Envelope_t* envelope){
	envelopeRegister(envelope);
	envelope->attack = 0;
	envelope->decay = 0;
	envelope->sustain = 0;
	envelope->release = 0;
}

float computeAttack(float attackPotentiometer){
	return 1.0f;
}

float getEnvelopeAttack(Envelope_t* envelope){
	return envelope->attack;
}

void setEnvelopeAttack(Envelope_t* envelope, float attack){
	envelope->attack = attack;
}
