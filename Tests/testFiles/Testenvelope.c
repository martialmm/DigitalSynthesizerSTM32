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

void envelope_with_a_release_of_0_second(){
	// Given
	float releaseTime = 0.0f;
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

void the_attack_is_stopped_during_its_rampup(){
	// Given
	float attackTime = 1.0f;
	float releaseTime = 1.0f;
	float output = 0.0f;
	setEnvelopeReleaseTime(envelope, releaseTime);
	setEnvelopeAttackTime(envelope, attackTime);

	/* ----- ATTACK PHASE ----- */

	// Note pressed
	setEnvelopeGate(envelope, 1);

	// When (1)
	// we want to stop the attack at its half:
	for(uint32_t i = 0; i < (uint32_t)(0.5f * getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	//Then (1)
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, output);


	/* ----- RELEASE PHASE ----- */

	// When (2)

	// note released
	setEnvelopeGate(envelope, 0);

	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeReleaseTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	//Then (2)
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output);
}

void the_attack_is_retriggered_while_release_rampdown(){
	// Given
	float attackTime = 1.0f;
	float releaseTime = 1.0f;
	float output = 1.0f;
	setEnvelopeAttackTime(envelope, attackTime);
	setEnvelopeReleaseTime(envelope, releaseTime);
	setEnvelopeCurrentLevel(envelope, 1.0f);


	/* ----- RELEASE PHASE ----- */

	// force envelope state
	setEnvelopeState(envelope, RELEASE);
	setEnvelopeGate(envelope, 0);

	// When (1)
	// we want to stop the release at its half:
	for(uint32_t i = 0; i < (uint32_t)(0.5f * getEnvelopeReleaseTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	//Then (1)
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, output);


	/* ----- ATTACK PHASE ----- */

	// Note pressed
	setEnvelopeGate(envelope, 1);

	// When (2)
	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	//Then (1)
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, output);
}

void envelope_with_a_decay_of_5_seconds(){
	// Given
	float decayTime = 5.0f;
	float output = 1.0f;

	setEnvelopeDecayTime(envelope, decayTime);
	setEnvelopeCurrentLevel(envelope, output);
	setEnvelopeGate(envelope, 1);
	setEnvelopeState(envelope, DECAY);

	// When
	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeDecayTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	// Then
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output);
}

void envelope_with_2_sec_attack_and_3_sec_decay(){
	// Given
	float attackTime = 2.0f;
	float decayTime = 3.0f;
	float output = 0.0f;

	setEnvelopeAttackTime(envelope, attackTime);
	setEnvelopeDecayTime(envelope, decayTime);

	/* ----- ATTACK PHASE ----- */
	setEnvelopeGate(envelope, 1);

	// When (1)
	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeAttackTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, output);

	/* ----- DECAY PHASE ----- */

	// When (2)
	for(uint32_t i = 0; i < (uint32_t)(getEnvelopeDecayTime(envelope) * SAMPLE_RATE); i++){
		output = processSampleEnvelope(envelope);
	}

	// Then
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output);
}


int main(void){
	UNITY_BEGIN();

	RUN_TEST(envelope_with_an_attack_of_1_second);
	RUN_TEST(envelope_with_an_attack_of_0_second);
	RUN_TEST(envelope_with_a_release_of_1_second);
	RUN_TEST(envelope_with_a_release_of_0_second);
	RUN_TEST(the_attack_is_stopped_during_its_rampup);
	RUN_TEST(the_attack_is_retriggered_while_release_rampdown);
	RUN_TEST(envelope_with_a_decay_of_5_seconds);
	RUN_TEST(envelope_with_2_sec_attack_and_3_sec_decay);

	return UNITY_END();
}
