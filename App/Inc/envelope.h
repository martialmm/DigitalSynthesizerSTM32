/*
 * envelope.h
 *
 *  Created on: May 13, 2026
 *      Author: mars
 */

#ifndef INC_ENVELOPE_H_
#define INC_ENVELOPE_H_

typedef struct Envelope Envelope_t;

Envelope_t* createEnvelope(void);
void envelopeRegister(Envelope_t* envelope);
void initEnvelope(Envelope_t* envelope);

#endif /* INC_ENVELOPE_H_ */
