#include <stdio.h>
#include "../Unity/src/unity.h"
#include "../../App/Inc/envelope.h"

static Envelope_t* envelope;

void setUp(){
    envelope = createEnvelope();
    initEnvelope(envelope);
}

void tearDown(){}

void envelope_with_an_attack_of_1_second(){
	// Given
	float attackTime = 1.0f; // in seconds
	float output = 0.0f;
	setEnvelopeGate(envelope, 1);
	setEnvelopeAttackTime(envelope, attackTime);

	// When
	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	// Then
	TEST_ASSERT_EQUAL_FLOAT(1.0f, output);
}

void envelope_with_an_attack_of_0_second(){
	// Given
	float attackTime = 0.0f; // in seconds
	float output = 0.0f;
	setEnvelopeGate(envelope, 1);
	setEnvelopeAttackTime(envelope, attackTime);

	// When
	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	// Then
	TEST_ASSERT_EQUAL_FLOAT(1.0f, output);
}


int main(void){
	UNITY_BEGIN();

	RUN_TEST(envelope_with_an_attack_of_1_second);
	RUN_TEST(envelope_with_an_attack_of_0_second);

	return UNITY_END();
}
