#pragma once
#include <functional>
#include <string>
#include <vector>

namespace S67 {

// Valve-style FCVAR flags
enum ConVarFlags {
  FCVAR_NONE = 0,
  FCVAR_UNREGISTERED = (1 << 0),
  FCVAR_ARCHIVE = (1 << 1),         // Save to config
  FCVAR_CHEAT = (1 << 2),           // Only allowed with sv_cheats 1
  FCVAR_REPLICATED = (1 << 3),      // Server -> Client
  FCVAR_USERINFO = (1 << 4),        // Client -> Server
  FCVAR_NOTIFY = (1 << 5),          // Notify peers of change
  FCVAR_PROTECTED = (1 << 6),       // Don't reveal to others
  FCVAR_PRINTABLEONLY = (1 << 7),   // Cannot contain unprintable chars
  FCVAR_UNLOGGED = (1 << 8),        // Don't log changes
  FCVAR_NEVER_AS_STRING = (1 << 9), // Optimization
};

class ConVar;
using ConVarChangeCallback =
    std::function<void(ConVar *var, const std::string &oldValue)>;

class ConVar {
public:
  ConVar(const std::string &name, const std::string &defaultValue,
         int flags = FCVAR_NONE, const std::string &helpString = "",
         ConVarChangeCallback callback = nullptr);
  ConVar(const std::string &name, const std::string &defaultValue, int flags,
         const std::string &helpString, bool hasMin, float min, bool hasMax,
         float max, ConVarChangeCallback callback = nullptr);

  ~ConVar();

  std::string GetName() const { return m_Name; }
  std::string GetHelpString() const { return m_HelpString; }
  int GetFlags() const { return m_Flags; }

  bool IsFlagSet(int flag) const { return (m_Flags & flag) != 0; }
  void AddFlags(int flags) { m_Flags |= flags; }
  void RemoveFlags(int flags) { m_Flags &= ~flags; }

  // Value Accessors
  int GetInt() const;
  float GetFloat() const;
  bool GetBool() const;
  std::string GetString() const;

  // Setters
  void SetValue(const std::string &value);
  void SetValue(int value);
  void SetValue(float value);

  void Revert(); // Reset to default

  bool HasMin() const { return m_HasMin; }
  bool HasMax() const { return m_HasMax; }
  float GetMin() const { return m_Min; }
  float GetMax() const { return m_Max; }

private:
  std::string m_Name;
  std::string m_DefaultValue;
  std::string m_StringValue;
  std::string m_HelpString;
  int m_Flags;

  // Cache values to avoid recurrent parsing?
  // For now, simplicity: parse on demand or cache on set.
  // Let's cache for performance similar to Source.
  int m_IntValue;
  float m_FloatValue;

  bool m_HasMin = false;
  float m_Min = 0.0f;
  bool m_HasMax = false;
  float m_Max = 0.0f;

  ConVarChangeCallback m_Callback;

  void InternalSetValue(const std::string &value);
};

} // namespace S67
