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
	uint16_t attack;
	uint16_t decay;
	uint16_t sustain;
	uint16_t release;
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
