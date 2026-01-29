#pragma once

#include "Core/Base.h"
#include <functional>
#include <string>
#include <vector>


namespace S67 {

// Forward declarations
class ConVar;
class ConCommand;

// Command flags
enum EConVarFlags : int {
  FCVAR_NONE = 0,
  FCVAR_UNREGISTERED = (1 << 0),     // If this is set, don't register
  FCVAR_ARCHIVE = (1 << 1),          // Save to config file
  FCVAR_CHEAT = (1 << 2),            // Only useable in cheat mode
  FCVAR_REPLICATED = (1 << 3),       // Server variable replicated to clients
  FCVAR_USERINFO = (1 << 4),         // Client variable sent to server
  FCVAR_NOTIFY = (1 << 5),           // Notify players when changed
  FCVAR_PROTECTED = (1 << 6),        // Don't send to clients (server-side only)
  FCVAR_SP_ONLY = (1 << 7),          // Singleplayer only
  FCVAR_PRINTABLEONLY = (1 << 8),    // Only printable characters
  FCVAR_UNLOGGED = (1 << 9),         // Don't log changes
  FCVAR_NEVER_AS_STRING = (1 << 10), // Never try to print as string
  FCVAR_RELOAD_MATERIALS = (1 << 11), // Reload materials when changed
  FCVAR_RELOAD_TEXTURES = (1 << 12),  // Reload textures when changed
  FCVAR_NOT_CONNECTED = (1 << 13), // Variable can't be changed while connected
  FCVAR_MATERIAL_SYSTEM_THREAD =
      (1 << 14),                  // Accessed by material system thread
  FCVAR_ARCHIVE_XBOX = (1 << 15), // Xbox specific
  FCVAR_ACCESSIBLE_FROM_THREADS = (1 << 16), // Thread safe
  FCVAR_SERVER_CAN_EXECUTE = (1 << 28),      // Server can execute this command
  FCVAR_SERVER_CANNOT_QUERY = (1 << 29),  // Server cannot query this variable
  FCVAR_CLIENTCMD_CAN_EXECUTE = (1 << 30) // Client can execute this command
};

// Callback type for when a ConVar changes
// void Callback(ConVar* var, const std::string& oldValue, float oldFloatValue);
using FnChangeCallback =
    std::function<void(ConVar *, const std::string &, float)>;

// -------------------------------------------------------------------------
// ConCommandBase
// Base class for both ConVars and ConCommands
// -------------------------------------------------------------------------
class ConCommandBase {
public:
  ConCommandBase(const char *name, const char *helpString = nullptr,
                 int flags = 0);
  virtual ~ConCommandBase();

  bool IsCommand() const;
  bool IsFlagSet(int flag) const;
  void AddFlags(int flags);
  void RemoveFlags(int flags);

  const char *GetName() const;
  const char *GetHelpString() const;
  int GetFlags() const;

  // Linked list management
  ConCommandBase *m_Next;
  static ConCommandBase *s_ConCommandBaseHead;

protected:
  const char *m_Name;
  const char *m_HelpString;
  int m_Flags;
  bool m_Registered;
};

// -------------------------------------------------------------------------
// ConVar
// Console Variable
// -------------------------------------------------------------------------
class ConVar : public ConCommandBase {
public:
  // Constructor for string default
  ConVar(const char *name, const char *defaultValue, int flags = 0,
         const char *helpString = nullptr, FnChangeCallback callback = nullptr);

  // Constructor with range validation
  ConVar(const char *name, const char *defaultValue, int flags,
         const char *helpString, bool hasMin, float min, bool hasMax, float max,
         FnChangeCallback callback = nullptr);

  virtual ~ConVar();

  // Accessors
  float GetFloat() const;
  int GetInt() const;
  bool GetBool() const;
  const char *GetString() const;

  // Setters
  void SetValue(const char *value);
  void SetValue(float value);
  void SetValue(int value);
  void SetValue(bool value);

  // Utilities
  void Revert();
  bool HasMin() const { return m_HasMin; }
  bool HasMax() const { return m_HasMax; }
  float GetMinValue() const { return m_MinVal; }
  float GetMaxValue() const { return m_MaxVal; }
  const char *GetDefault() const { return m_DefaultValue.c_str(); }

private:
  void Init();
  void InternalSetValue(const std::string &value);
  void ChangeValue(const std::string &tempVal, float flOldValue);

private:
  std::string m_Value;
  std::string m_DefaultValue;
  float m_FloatValue;
  int m_IntValue;
  bool m_HasMin;
  float m_MinVal;
  bool m_HasMax;
  float m_MaxVal;

  FnChangeCallback m_ChangeCallback;
};

} // namespace S67
