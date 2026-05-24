#include <stdio.h>
#include "../Unity/src/unity.h"
#include "../../App/Inc/envelope.h"

void setUp(){}
void tearDown(){}

void envelope_with_an_attack_of_4_seconds(){

}

int main(void){
	UNITY_BEGIN();

	RUN_TEST(envelope_with_an_attack_of_4_seconds);

	return UNITY_END();
}
