#include "unity.h"
#include "isoCards.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_function_should_Serialize_Card(void) {
    ISO_CARD cardToSerialize ={
        245,
        "Hello",
        {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
        6,
        5,
        true
    };
    Serial.println("Testing Serialize");
    Serial.println(cardToString(cardToSerialize).c_str());

    Serial.println("Testing Serialize with slotID");
    Serial.println(cardToString(cardToSerialize, true).c_str());
}
int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_should_Serialize_Card);
  return UNITY_END();
}

/**
  * For Arduino framework
  */
void setup() {
  Serial.begin(115200);
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}
void loop() {}