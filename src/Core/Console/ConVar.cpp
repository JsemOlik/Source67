#include "ConVar.h"
#include "Core/Console/Console.h"
#include "Core/Logger.h"

#include <algorithm>
#include <cstdlib>

namespace S67 {

// -------------------------------------------------------------------------
// ConCommandBase
// -------------------------------------------------------------------------
ConCommandBase *ConCommandBase::s_ConCommandBaseHead = nullptr;

ConCommandBase::ConCommandBase(const char *name, const char *helpString,
                               int flags)
    : m_Name(name), m_HelpString(helpString), m_Flags(flags),
      m_Registered(false) {
  // Link into global list
  m_Next = s_ConCommandBaseHead;
  s_ConCommandBaseHead = this;
  m_Registered = true;
}

ConCommandBase::~ConCommandBase() {
  // Unlinking usually handled by system shutdown or not needed if static,
  // but for dynamic cvar dynamic unloading might be needed.
  // For now, simple implementation assuming static duration mostly.
}

bool ConCommandBase::IsCommand() const {
  // ConVar overrides this? No, we check via dynamic cast or lightweight RTTI if
  // needed But standard Source way is just a virtual method returning
  // false/true We haven't added that pure virtual yet, let's assume false and
  // ConCommand will override
  return false;
}

bool ConCommandBase::IsFlagSet(int flag) const { return (m_Flags & flag) != 0; }

void ConCommandBase::AddFlags(int flags) { m_Flags |= flags; }

void ConCommandBase::RemoveFlags(int flags) { m_Flags &= ~flags; }

const char *ConCommandBase::GetName() const { return m_Name; }

const char *ConCommandBase::GetHelpString() const { return m_HelpString; }

int ConCommandBase::GetFlags() const { return m_Flags; }

// -------------------------------------------------------------------------
// ConVar
// -------------------------------------------------------------------------

ConVar::ConVar(const char *name, const char *defaultValue, int flags,
               const char *helpString, FnChangeCallback callback)
    : ConCommandBase(name, helpString, flags) {
  m_DefaultValue = defaultValue ? defaultValue : "";
  m_HasMin = false;
  m_MinVal = 0.0f;
  m_HasMax = false;
  m_MaxVal = 0.0f;
  m_ChangeCallback = callback;
  Init();
}

ConVar::ConVar(const char *name, const char *defaultValue, int flags,
               const char *helpString, bool hasMin, float min, bool hasMax,
               float max, FnChangeCallback callback)
    : ConCommandBase(name, helpString, flags) {
  m_DefaultValue = defaultValue ? defaultValue : "";
  m_HasMin = hasMin;
  m_MinVal = min;
  m_HasMax = hasMax;
  m_MaxVal = max;
  m_ChangeCallback = callback;
  Init();
}

ConVar::~ConVar() {}

void ConVar::Init() { InternalSetValue(m_DefaultValue); }

void ConVar::InternalSetValue(const std::string &value) {
  float fValue = (float)atof(value.c_str());

  // Clamp
  if (m_HasMin && fValue < m_MinVal)
    fValue = m_MinVal;
  if (m_HasMax && fValue > m_MaxVal)
    fValue = m_MaxVal;

  m_FloatValue = fValue;
  m_IntValue = (int)fValue;

  // In Source, if we clamp, the string value might differ from 'value'
  // But typically we store what we have.
  // Let's store the clamped string if it was clamped, or just the value if not
  // (or close enough)

  // Simple approach: Store what was converted (handles clamping string sync)
  if (value.find('.') != std::string::npos || m_HasMin || m_HasMax) {
    // It might be a float, re-serialize to be safe?
    // Or just trust if it wasn't clamped?
    // Let's just store 'value' if not clamped, else form new string
    bool clamped = (m_HasMin && (float)atof(value.c_str()) < m_MinVal) ||
                   (m_HasMax && (float)atof(value.c_str()) > m_MaxVal);
    if (clamped) {
      m_Value = std::to_string(m_FloatValue);
      // remove trailing zeros for cleanliness? optional
      m_Value.erase(m_Value.find_last_not_of('0') + 1, std::string::npos);
      if (m_Value.back() == '.')
        m_Value.pop_back();
    } else {
      m_Value = value;
    }
  } else {
    m_Value = value;
  }
}

void ConVar::SetValue(const char *value) {
  std::string sValue = value ? value : "";
  std::string oldValue = m_Value;
  float oldFloat = m_FloatValue;

  InternalSetValue(sValue);

  if (m_ChangeCallback && (m_Value != oldValue)) {
    m_ChangeCallback(this, oldValue, oldFloat);
  }
}

void ConVar::SetValue(float value) {
  std::string oldValue = m_Value;
  float oldFloat = m_FloatValue;

  // Clamp
  if (m_HasMin && value < m_MinVal)
    value = m_MinVal;
  if (m_HasMax && value > m_MaxVal)
    value = m_MaxVal;

  m_FloatValue = value;
  m_IntValue = (int)value;
  m_Value = std::to_string(value);
  // Clean string
  m_Value.erase(m_Value.find_last_not_of('0') + 1, std::string::npos);
  if (m_Value.back() == '.')
    m_Value.pop_back();

  if (m_ChangeCallback && (m_Value != oldValue)) {
    m_ChangeCallback(this, oldValue, oldFloat);
  }
}

void ConVar::SetValue(int value) { SetValue((float)value); }

void ConVar::SetValue(bool value) { SetValue(value ? 1.0f : 0.0f); }

float ConVar::GetFloat() const { return m_FloatValue; }
int ConVar::GetInt() const { return m_IntValue; }
bool ConVar::GetBool() const { return !!m_IntValue; }
const char *ConVar::GetString() const { return m_Value.c_str(); }

void ConVar::Revert() { SetValue(m_DefaultValue.c_str()); }

} // namespace S67
