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

	setEnvelopeState(envelope, ATTACK);

	// When
	for(uint32_t i = 0; i <= (uint32_t)(getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	// Then
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, output);
}

void envelope_with_an_attack_of_0_second(){
	// Given
	float attackTime = 0.0f; // in seconds
	float output = 0.0f;
	setEnvelopeGate(envelope, 1);
	setEnvelopeAttackTime(envelope, attackTime);

	setEnvelopeState(envelope, ATTACK);

	// When
	for(uint32_t i = 0; i <= (uint32_t)(getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	// Then
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, output);
}

void envelope_with_a_release_of_1_second(){
	// Given
	float releaseTime = 1.0f;
	float output = 1.0f;
	setEnvelopeReleaseTime(envelope, releaseTime);
	setEnvelopeCurrentLevel(envelope, 1.0f);
	setEnvelopeGate(envelope, 0);

	// force envelope state
	setEnvelopeState(envelope, RELEASE);

	// When
	for(uint32_t i = 0; i <= (uint32_t)(getEnvelopeReleaseTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	//Then
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output);
}


int main(void){
	UNITY_BEGIN();

	RUN_TEST(envelope_with_an_attack_of_1_second);
	RUN_TEST(envelope_with_an_attack_of_0_second);
	RUN_TEST(envelope_with_a_release_of_1_second);

	return UNITY_END();
}
