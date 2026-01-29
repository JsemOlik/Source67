#include "ConVar.h"
#include "Console.h"
#include "Core/Logger.h"
#include <algorithm>

namespace S67 {

ConVar::ConVar(const std::string &name, const std::string &defaultValue,
               int flags, const std::string &helpString,
               ConVarChangeCallback callback)
    : m_Name(name), m_DefaultValue(defaultValue), m_Flags(flags),
      m_HelpString(helpString), m_Callback(callback) {
  InternalSetValue(defaultValue);
  Console::Get().RegisterConVar(this);
}

ConVar::ConVar(const std::string &name, const std::string &defaultValue,
               int flags, const std::string &helpString, bool hasMin, float min,
               bool hasMax, float max, ConVarChangeCallback callback)
    : m_Name(name), m_DefaultValue(defaultValue), m_Flags(flags),
      m_HelpString(helpString), m_HasMin(hasMin), m_Min(min), m_HasMax(hasMax),
      m_Max(max), m_Callback(callback) {
  InternalSetValue(defaultValue);
  Console::Get().RegisterConVar(this);
}

ConVar::~ConVar() { Console::Get().UnregisterConVar(this); }

int ConVar::GetInt() const { return m_IntValue; }

float ConVar::GetFloat() const { return m_FloatValue; }

bool ConVar::GetBool() const { return m_IntValue != 0; }

std::string ConVar::GetString() const { return m_StringValue; }

void ConVar::SetValue(const std::string &value) {
  std::string oldValue = m_StringValue;
  InternalSetValue(value);
  if (m_Callback && oldValue != m_StringValue) {
    m_Callback(this, oldValue);
  }
}

void ConVar::SetValue(int value) { SetValue(std::to_string(value)); }

void ConVar::SetValue(float value) { SetValue(std::to_string(value)); }

void ConVar::Revert() { SetValue(m_DefaultValue); }

void ConVar::InternalSetValue(const std::string &value) {
  // Parse float first to handle clamping
  float fVal = 0.0f;
  try {
    fVal = std::stof(value);
  } catch (...) {
    fVal = 0.0f; // Default if parse fails? Or just keep 0.
  }

  if (m_HasMin && fVal < m_Min)
    fVal = m_Min;
  if (m_HasMax && fVal > m_Max)
    fVal = m_Max;

  // If clamping happened, update string representation
  bool clamped = (m_HasMin && std::stof(value) < m_Min) ||
                 (m_HasMax && std::stof(value) > m_Max);

  if (clamped ||
      (value.find('.') != std::string::npos && value.back() == '0')) {
    // Re-serialize if clamped or if we want to normalize?
    // Source keeps string if possible, but for clamping we must update it
    // Let's just store the string provided unless clamped
  }

  if (clamped) {
    m_FloatValue = fVal;
    m_IntValue = static_cast<int>(fVal);
    // Remove trailing zeros for cleanliness
    std::string s = std::to_string(fVal);
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.')
      s.pop_back();
    m_StringValue = s;
  } else {
    m_StringValue = value;
    m_FloatValue = fVal;
    m_IntValue = static_cast<int>(fVal);
  }
}

} // namespace S67
