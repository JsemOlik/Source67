# Developer Console System Research

## Source Engine Console Architecture

### Historical Context
Valve's Source Engine (used in Half-Life 2, Counter-Strike: Source, Team Fortress 2, Portal, Portal 2, CS:GO, CS2) implements a sophisticated console variable system that has become industry standard for game development.

## Core Classes & Structure

### ConVar (Console Variable)
ConVar is the base class for all console variables in the Source engine.

**Constructor signature:**
```cpp
ConVar(
    const char *pName,              // Variable name (e.g., "sv_gravity")
    const char *pDefaultValue,      // Default value as string
    int flags = 0,                  // FCVAR_* bitflags
    const char *pHelpString = 0,    // Description for help text
    bool hasMin = false,            // Is there a minimum value?
    float minValue = 0,             // Minimum value
    bool hasMax = false,            // Is there a maximum value?
    float maxValue = 1.0,           // Maximum value
    FnChangeCallback callback = 0   // Callback function pointer
);
```

**Key Methods:**
```cpp
// Type-specific getters (all store internally as string, convert on access)
int GetInt() const;
float GetFloat() const;
const char* GetString() const;
bool GetBool() const;  // Returns true if GetInt() != 0

// Type-agnostic setters (accept any type, convert to string)
void SetValue(int value);
void SetValue(float value);
void SetValue(const char *value);

// Utility methods
const char* GetName() const;
const char* GetHelpString() const;
void Revert();  // Reset to default value
bool IsValid() const;
void AddFlags(int flags);  // Add flags after creation
bool IsFlagSet(int flag) const;
```

**Memory consideration:**
- ConVars are typically declared as global or static variables
- Constructor auto-registers the variable with the console system
- This happens at module load time, not runtime
- All ConVars form a linked list internally

### ConCommand (Console Command)
ConCommand is for commands that execute code rather than store values.

**Constructor signature:**
```cpp
ConCommand(
    const char *pName,              // Command name
    FnCommandCallback callback,      // Function to call when invoked
    const char *pHelpString = 0,    // Description
    int flags = 0,                  // FCVAR_* bitflags
    FnCommandCompletionCallback completionFunc = 0  // Auto-complete provider
);
```

**Callback function signature:**
```cpp
void MyCommand_f(const CCommand &args) {
    // args.ArgC() - number of arguments
    // args.Arg(0) - command name itself
    // args.Arg(1) - first argument
    // args.Arg(2) - second argument, etc.
    // All arguments are strings
}
```

### ConCommandBase (Abstract Base)
Both ConVar and ConCommand inherit from ConCommandBase:
```
ConCommandBase
├── ConVar
└── ConCommand
```

Provides:
- Name storage and lookup
- Flag management
- Help text storage
- Registration in global command list

### ICvar Interface
The engine provides access to all registered ConVars/ConCommands through the ICvar interface:

**Key functions:**
```cpp
// Find a specific variable by name
ConVar *pVar = cvar->FindVar("sv_gravity");

// Get value (if you have a pointer)
int gravity = pVar->GetInt();

// Iterate all variables (for debugging)
for (ICvarIteratorInternal *iter = ...; iter->IsValid(); iter->Next()) {
    ConCommandBase *base = iter->GetBase();
    // ...
}

// Find and create callback
void OnCvarChanged(IConVar *var, const char *pOldValue, float flOldValue) {
    DevMsg("Cvar changed: %s from %s to %s\n", 
        var->GetName(), pOldValue, var->GetString());
}
```

## FCVAR Flags Deep Dive

Flags are defined in `src/public/tier1/iconvar.h` with `FCVAR_` prefix. They are bitwise flags that can be combined.

### Flag Categories

#### Access Control Flags
| Flag | Effect | Use Case |
|------|--------|----------|
| `FCVAR_CHEAT` | Requires `sv_cheats 1` in multiplayer | God mode, noclip, spawn items |
| `FCVAR_PROTECTED` | Server cannot query value from clients | Passwords, private client settings |
| `FCVAR_SERVER_CANNOT_QUERY` | Inverse of PROTECTED | Client settings to hide from server |
| `FCVAR_GAMEDLL` | Only set by game DLL | Server-side game logic |
| `FCVAR_CLIENTDLL` | Only set by client DLL | Client-side game logic |

#### Persistence & Sync Flags
| Flag | Effect | Use Case |
|------|--------|----------|
| `FCVAR_ARCHIVE` | Saved to config.cfg | User settings, preferences |
| `FCVAR_REPLICATED` | Server → all clients | Game rules, physics values |
| `FCVAR_USERINFO` | Client → server | Player name, team, settings |
| `FCVAR_NOTIFY` | Print to chat when changed | Important game state changes |
| `FCVAR_SPAWNFLAGS` | Used in entity spawn | Map entity properties |

#### Behavioral Flags
| Flag | Effect | Use Case |
|------|--------|----------|
| `FCVAR_NEVER_AS_STRING` | Force typed access | Performance optimization |
| `FCVAR_RELEASE` | Only in release builds | Debug-only variables |
| `FCVAR_RELOAD_MATERIALS` | Triggers material reload | Texture/shader changes |
| `FCVAR_RELOAD_TEXTURES` | Triggers texture reload | Texture changes |
| `FCVAR_DEMO` | Affects demo playback | Demo-specific behavior |
| `FCVAR_DONTRECORD` | Not recorded in demos | Temporary settings |

#### Execution Flags
| Flag | Effect | Use Case |
|------|--------|----------|
| `FCVAR_SERVER_CAN_EXECUTE` | Server can run client command | Server-forced config |
| `FCVAR_CLIENTCMD_CAN_EXECUTE` | Client can run via IVEngineClient | Client-side command execution |

### Flag Bitwise Combinations
```cpp
// Single flag
ConVar sv_gravity("sv_gravity", "800", FCVAR_ARCHIVE, "...");

// Multiple flags
ConVar mp_friendlyfire("mp_friendlyfire", "0", 
    FCVAR_NOTIFY | FCVAR_REPLICATED, "...");

// Complex combination
ConVar cl_name("name", "Player", 
    FCVAR_ARCHIVE | FCVAR_USERINFO | FCVAR_NEVER_AS_STRING, "...");
```

## Variable Naming Conventions in Source Games

### CS:GO / CS2 Examples
```
Client-side (cl_):
- cl_cmdrate (30-128) - command rate to server
- cl_updaterate (30-128) - update rate from server  
- cl_smooth (0-1) - smooth out prediction errors
- cl_crosshairalpha (0-255) - crosshair opacity
- cl_autowepswitch (0-1) - auto weapon switch
- cl_team (0-3) - preferred team
- cl_backspeed - backwards movement speed
- cl_forwardspeed - forward movement speed
- cl_sidespeed - strafe speed

Server-side (sv_):
- sv_gravity (0-1000) - world gravity
- sv_cheats (0-1) - enable cheats
- sv_maxrate - maximum bandwidth per client
- sv_maxplayers - server capacity
- sv_password - server password (empty = public)
- sv_maxrounde (0-65535) - rounds per match
- sv_roundtime - round duration in minutes
- sv_freezetime - freeze period at round start
- sv_buy_status - allow/disallow buying

Gameplay rules (mp_):
- mp_friendlyfire (0-1) - friendly fire
- mp_roundtime - round time
- mp_timelimit - map time limit
- mp_startmoney - starting player money
- mp_buytime - buy phase duration

Rendering (r_):
- r_drawparticles (0-1)
- r_shadows (0-1)
- r_skybox (0-1)
- r_dynamic (0-1) - dynamic lighting
```

### Team Fortress 2 Examples
```
Gameplay mechanics:
- tf_max_health_boost (1.0-5.0) - max overheal multiplier
- tf_bot_quota (0+) - number of bots
- tf_damage_disablespawn_time (0+) - spawn invulnerability
- sv_smart_sentry_gun_placement (0-1)
- sv_allow_point_servercommand (0-1)
- sv_allow_vote_spawnroom_lock (0-1)

Game rules:
- mp_tournament (0-1) - tournament mode
- mp_stalemate_enable (0-1) - stalemate rounds
- mp_stalemate_timelimit (0+) - stalemate duration
```

## Callback System

When ConVar values change, callbacks can be triggered:

```cpp
// Define callback
static void OnGravityChange(IConVar *var, const char *pOldValue, float flOldValue) {
    ConVar *gravity = (ConVar *)var;
    Msg("Gravity changed from %s to %s\n", pOldValue, gravity->GetString());
    
    // Update physics system
    g_Physics.SetGravity(gravity->GetFloat());
}

// Register with callback
ConVar sv_gravity("sv_gravity", "800", FCVAR_ARCHIVE, 
    "World gravity", OnGravityChange);
```

**Callback signature:**
- First parameter: `IConVar *var` - the variable that changed (cast to `ConVar*` if needed)
- Second parameter: `const char *pOldValue` - string representation of old value
- Third parameter: `float flOldValue` - old value as float (for convenience)

## Validation & Range Checking

ConVars can enforce minimum and maximum values:

```cpp
// Range validation constructor
ConVar cl_cmdrate("cl_cmdrate", "30", 0, "Command rate",
    true, 1.0f,     // Has minimum? Yes, value 1.0
    true, 128.0f    // Has maximum? Yes, value 128.0
);

// If user types: cl_cmdrate 200
// Engine automatically clamps to 128.0
// No callback needed for automatic enforcement
```

**Implementation detail:** Range checking happens in the console system before the value is actually set, so invalid values are silently clamped rather than rejected.

## Config File Structure

ConVars marked with `FCVAR_ARCHIVE` are saved to `config.cfg`:

```cfg
// Typical config.cfg from Source engine game
// Auto-generated by engine on shutdown
// Loaded on game start

// Client network settings
cl_cmdrate "128"
cl_updaterate "128"
rate "786432"

// Graphics settings
r_drawparticles "1"
r_shadows "1"
sensitivity "2.5"

// Game preferences
name "PlayerName"
cl_team "0"
cl_backspeed "400"
cl_forwardspeed "450"

// custom server.cfg (loaded by server)
sv_gravity "800"
sv_cheats "0"
sv_maxplayers "32"
mp_friendlyfire "0"
mp_roundtime "5"
```

Files are loaded with `exec config.cfg` command, which parses and executes each line as a console command.

## Implementation Challenges & Solutions

### Challenge 1: Type Coercion
ConVars store values as strings internally but need to be accessible as int/float/bool.

**Solution:** Type conversion happens at access time:
```cpp
// Stored as: "42"
GetInt()      // atoi("42") → 42
GetFloat()    // atof("42") → 42.0f
GetString()   // "42"
GetBool()     // (atoi("42") != 0) → true
```

### Challenge 2: Flag Enforcement
Different flags impose different access restrictions at different times.

**Solution:** Check flags in:
1. **At parse time**: Reject cheat commands if `sv_cheats == 0`
2. **At access time**: Check `FCVAR_PROTECTED` before querying
3. **At modification time**: Check all relevant flags before allowing change

### Challenge 3: Replication
Server changes to replicated cvars must sync to all clients.

**Solution:** 
- Keep replicated cvar changes in a network message queue
- Send updates on next network tick
- Clients apply forced updates (cannot reject)
- Replicated cvars are read-only for clients

### Challenge 4: Callback Timing
When should callbacks fire relative to the actual value change?

**Solution:** Source engine fires callbacks AFTER value change, allowing:
- Reading both old value (passed as parameter)
- Reading new value (calling GetInt/Float/String)
- Making dependent updates based on new value

### Challenge 5: Default Value Restoration
`Revert()` should reset to compile-time default without losing current flag/range info.

**Solution:** Store default string separately from current value:
```cpp
class ConVar {
    const char *m_pszDefaultValue;  // Compile-time default
    const char *m_pszCurrentValue;  // Current value
    
    void Revert() {
        SetValue(m_pszDefaultValue);
    }
};
```

## Real-World Usage Patterns

### Pattern 1: Physics-Dependent Variable
```cpp
static void OnGravityChanged(IConVar *var, const char *pOld, float flOld) {
    // Update physics engine
    PhysicsSystem::SetGravity(((ConVar*)var)->GetFloat());
}

ConVar sv_gravity("sv_gravity", "800.0", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "World gravity constant", OnGravityChanged);
```

### Pattern 2: Cheat-Protected Rendering
```cpp
ConVar god_mode("god", "0", FCVAR_CHEAT, "God mode");

// In game code:
if (god_mode.GetInt()) {
    player->TakeDamage(0);  // Invulnerable
}
```

### Pattern 3: Server Config (server.cfg)
```
// server.cfg - loaded once at server startup
sv_cheats 0
sv_gravity 800
sv_maxplayers 32
mp_friendlyfire 0
mp_roundtime 5
sv_password ""
hostname "My Game Server"
```

### Pattern 4: Per-Client Settings (USERINFO)
```cpp
ConVar name("name", "UnnamedPlayer", FCVAR_ARCHIVE | FCVAR_USERINFO,
    "Player name");

// When player changes name, server is notified:
// - Old player info: name "OldName"
// - New player info: name "NewName"
// - Server updates scoreboard
// - Other players see the change
```

## Common Tweaks in Popular Games

### Competitive CS:GO / CS2 Server Settings
```
sv_cheats 0
sv_pure 2  // File integrity check
sv_gravity 800
sv_air_accel 12
sv_friction 5.2
sv_accelerate 5.5
mp_friendlyfire 0
mp_maxmoney 16000
mp_startmoney 800
mp_buytime 40
mp_freezetime 15
mp_roundtime 1.92
mp_timelimit 0
sv_maxrate 0  // Unlimited
sv_minrate 196608  // Minimum 192 kb/s
```

### Competitive TF2 Server Settings
```
sv_cheats 0
sv_gravity 800
mp_tournament 1
mp_tournament_stopwatch 1
mp_stalemate_enable 0
tf_damage_disablespawn_time 17
sv_smart_sentry_gun_placement 1
sv_allow_point_servercommand "online"
sv_allow_vote_spawnroom_lock 0
sv_allow_vote_kick 1
```

### Casual / Community Settings
```
sv_cheats 1  // Allow all commands
sv_gravity 400-1200  // Custom gravity
tf_max_health_boost 3.0  // More overheal
sv_timescale 0.5-2.0  // Time scale for practice
sv_infinite_ammo 1  // For testing
god 1  // Invulnerability
noclip 1  // No collision
```

## Engine Modules & Variable Distribution

Different modules register their own cvars:

- **Engine**: `sv_gravity`, `sv_maxrate`, `sv_tickrate`, `host_timescale`
- **Client**: `cl_cmdrate`, `cl_updaterate`, `sensitivity`, `cl_smooth`
- **Game DLL (Server)**: `mp_friendlyfire`, `sv_damage_scale`, gameplay rules
- **Client DLL**: `cl_drawparticles`, graphics settings
- **Material System**: `r_shadows`, `r_drawdecals`, rendering options
- **Physics Engine**: `sv_gravity`, `sv_friction`, `sv_acceleration`

Each module includes `<convar.h>` and registers its variables at module load time.

## Testing & Debugging

Commands for console testing:
```
cvarlist                    // Print all cvars
cvarlist sv_                // Print cvars starting with sv_
cvarlist cl_ 10             // Print first 10 cl_ cvars
find sv_gravity             // Find variable containing "sv_gravity"
help sv_gravity             // Show help text
sv_gravity 400              // Change value
sv_gravity                  // Display current value
cvar_find sv_gravity        // Search with pattern matching
```

Network debugging:
```
net_graph 1                 // Show network graph
net_showfragments 1         // Show network fragments
sv_showimpacts 1            // Show bullet impacts (requires sv_cheats)
cl_drawshotgunpellets 1     // Debug shooting (cheat)
```

## Performance Considerations

1. **Cvar access cost**: String conversion happens every access, so:
   - Cache ConVar pointers: `ConVar *pGravity = cvar->FindVar("sv_gravity");`
   - Access infrequently (not every frame)
   - Or cache the value locally: `float gravity = pGravity->GetFloat();`

2. **Registration overhead**: All variables registered at load time in linked list:
   - Lookups are O(n) without optimization
   - Consider implementing hash table for >100 variables

3. **Network sync cost**: Replicated variables send network packets:
   - Minimize frequency of changes
   - Batch changes into single network message
   - Only replicate when value actually changes

4. **Callback overhead**: Each cvar change triggers callback:
   - Keep callbacks simple and fast
   - Don't do heavy operations in callbacks
   - Cache relevant data before value change
