#ifndef Game_NLP
#define Game_NLP

#include "Common.h"
#include "LTE/String.h"

String AngularVelocityToString(float av);

String DistanceToString(float distance);

String DurationToString(float duration);

inline String DurationToString(gametime_t duration) {
  return DurationToString((float)duration / (float)kTimeScale);
}

String MassToString(Mass mass);

String RofToString(float rof);

String PhraseToSentence(String const& phrase);

String VelocityToString(float velocity);

String QuantityToString(Quantity quantity);

String QuantityToString(float quantity);

String QuantityToStringExact(Quantity quantity);

String ScaleToClass(float scale);

String String_Time(gametime_t time);

#endif
