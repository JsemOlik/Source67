# Developer Console Implementation Prompt

## Overview
You are tasked with implementing a developer console system for a C++ OpenGL game engine, modeled after the Valve Source Engine console architecture (as seen in CS2, CS:GO, Portal, Portal 2, TF, TF2).

## Core Architecture Requirements

### ConVar System
Implement a `ConVar` (Console Variable) class that:
- Stores variables that can be altered through the console
- Can be modified at runtime without recompilation
- Can be persisted to a config file (config.cfg)
- Supports multiple data types (int, float, string)
- Supports callbacks when values are changed
- Can have min/max range validation

**Example structure needed:**
```cpp
ConVar sv_gravity( "sv_gravity", "800", FCVAR_ARCHIVE, "Gravity amount" );
ConVar cl_cmdrate( "cl_cmdrate", "30", 0, "Packet rate to server" );
```

### ConCommand System
Implement a `ConCommand` class that:
- Executes a callback function when invoked
- Can accept command-line arguments
- Supports help text and auto-completion
- Supports various flags to control behavior

**Example structure needed:**
```cpp
void MyCommand_f( const CCommand &args ) { /* ... */ }
ConCommand my_command( "my_command", MyCommand_f, "Help text", FCVAR_CHEAT );
```

## Console Variable Naming Conventions

All console variables follow strict naming patterns that indicate their scope and function:

### Prefix System
- **`cl_`** = Client-side variable (player's local copy)
  - Examples: `cl_cmdrate`, `cl_updaterate`, `cl_smooth`, `cl_crosshairalpha`
  - Only affects the local client, cannot be synchronized to server
  
- **`sv_`** = Server-side variable (server authority)
  - Examples: `sv_gravity`, `sv_cheats`, `sv_maxrate`, `sv_maxplayers`
  - Controls game rules and must be enforced by server
  
- **`r_`** = Rendering/Graphics variables
  - Examples: `r_drawparticles`, `r_skybox`, `r_shadows`
  - Affects visual output, client-side only
  
- **`net_`** = Network diagnostic variables
  - Examples: `net_graph`, `net_showconsole`
  - Used for debugging network issues

- **`ui_`, `hud_`, `con_`** = UI/HUD/Console-specific variables
  - Control user interface elements

### Naming Pattern Structure
Variables are **always lowercase with underscores**, no spaces:
- `sv_gravity` (correct)
- `SvGravity` or `sv-gravity` (incorrect)

## Variable Data Types

ConVars are **not explicitly typed** in the Source engine. Instead, they store string values internally and are accessed via type-specific getters:

### Data Type Access Methods
```cpp
ConVar my_var("my_var", "42", ...);

// Access as different types
int value_int = my_var.GetInt();              // Returns integer (42)
float value_float = my_var.GetFloat();        // Returns float (42.0)
const char* value_str = my_var.GetString();   // Returns string ("42")

// Set values (all types convert to string internally)
my_var.SetValue(42);
my_var.SetValue(42.0f);
my_var.SetValue("42");
```

### Common Type Patterns
- **Booleans**: Stored as 0 or 1, accessed via `GetInt()` or comparison
  - `if ( my_var.GetInt() == 1 ) { ... }`
  
- **Floats**: Physics values, timing, visual settings
  - `sv_gravity`, `host_timescale`, `cl_smooth`
  
- **Integers**: Counters, ports, maximum values
  - `sv_maxplayers`, `cl_cmdrate`, `sv_port`
  
- **Strings**: Configuration, names, file paths
  - `hostname`, `sv_password`, `cl_team`

## FCVAR Flags System

Flags control how ConVars behave and who can modify them. Flags are bitflags prefixed with `FCVAR_`:

### Common Flags

**`FCVAR_CHEAT`**
- In multiplayer: variable cannot be changed unless `sv_cheats 1` is set
- Prevents cheating by restricting access to variables that give unfair advantage
- Has no effect in singleplayer (unless explicitly checked in code)
- Examples: `noclip`, `god`, `fly`, `r_drawparticles`

**`FCVAR_ARCHIVE`**
- Value is saved to `config.cfg` when game shuts down
- Value is restored from `config.cfg` on game restart
- Use for user-configurable settings that should persist
- Examples: `cl_cmdrate`, `sensitivity`, `volume`, `name`
- Note: Avoid combining with `FCVAR_CHEAT` as it can cause unexpected resets

**`FCVAR_REPLICATED`**
- Server forces all connected clients to use the same value
- Essential for shared game code (movement, weapon mechanics, game rules)
- Examples: `sv_gravity`, `mp_friendlyfire`, `mp_timelimit`
- Both client and server must run exact same code path with same data

**`FCVAR_USERINFO`**
- Client-side variable that is sent to server
- Server receives updates whenever player changes the value
- Used for client settings the server needs to know about
- Examples: `name`, `cl_team`, `rate`, `model`
- Server can query via `GetClientConVarValue()`

**`FCVAR_NOTIFY`**
- Server announces to all players when this variable changes
- Use for variables affecting gameplay that players should know about
- Examples: `mp_friendlyfire`, `mp_timelimit`, `mp_roundtime`
- Printed to chat like: `"mp_friendlyfire" changed to "1"`

**`FCVAR_SERVER_CAN_EXECUTE`**
- Allows server to execute client-side commands
- Without this flag, server execution is blocked
- Useful for commands the server needs to run on clients
- Example: server-forced config commands

### Flag Combinations (Examples)
```cpp
// Player name - persists and sends to server
ConVar name("name", "Player", FCVAR_ARCHIVE | FCVAR_USERINFO, "Player name");

// Game rule that affects all - replicated and notified
ConVar mp_friendlyfire("mp_friendlyfire", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "...");

// Graphics setting - archived but client-only
ConVar r_shadows("r_shadows", "1", FCVAR_ARCHIVE, "Enable shadows");

// Cheat variable - restricted unless sv_cheats enabled
ConVar god("god", "0", FCVAR_CHEAT, "God mode");
```

## Classic Gameplay Variables Reference

These are the most commonly tweaked variables in Valve games that you should support:

### Physics & Movement
- `sv_gravity` - Gravity strength (default 800)
- `sv_acceleration` - Player movement acceleration
- `sv_airaccelerate` - Air strafing acceleration
- `sv_friction` - Ground friction
- `sv_timescale` - Game speed multiplier (0.5 = slow, 2.0 = fast)

### Weapon & Combat
- `sv_damage_scale` - Damage multiplier
- `sv_friendly_damage` - Friendly fire damage percentage
- `mp_friendlyfire` - Enable/disable friendly fire (0 or 1)
- `mp_damagescale` - Overall damage multiplier

### Networking & Performance
- `cl_cmdrate` - Packets sent to server per second (typical: 30, modern: 105)
- `cl_updaterate` - Updates received from server per second (typical: 20-66)
- `sv_maxrate` - Maximum bandwidth server sends per client
- `rate` - Maximum bandwidth client receives from server
- `net_tickrate` or `sv_tickrate` - Server tick rate (typically 64 or 128)
- `host_timescale` - Time scale (1.0 = normal, affects physics)

### Graphics
- `r_drawparticles` - Show particle effects
- `r_shadows` - Show dynamic shadows
- `r_skybox` - Show sky box
- `cl_bob` - View bob amount while moving
- `cl_smooth` - Smooth prediction errors
- `cl_smoothtime` - Duration of smoothing (in seconds)

### Game Rules
- `sv_cheats` - Enable cheat commands (0 or 1)
- `sv_maxplayers` - Maximum player count
- `sv_allow_wait_command` - Allow wait command in configs
- `mp_timelimit` - Round time limit in minutes
- `mp_roundtime` - Single round duration

### Client Preferences
- `cl_team` - Default team when joining
- `cl_autoaim` - Auto-aim assistance (legacy)
- `cl_autowepswitch` - Auto-switch weapons when picking up
- `cl_backspeed` - Backward movement speed
- `cl_forwardspeed` - Forward movement speed
- `cl_sidespeed` - Strafe movement speed
- `sensitivity` - Mouse sensitivity

### Console & Debug
- `developer` - Debug message level (0, 1, or 2)
- `sv_cheats` - Enables most debug commands
- `con_filter_enable` - Filter console output
- `con_filter_text` - Filtered text pattern

## Implementation Requirements Checklist

Your AI should implement:

- [ ] `ConVar` class with string-based internal storage
- [ ] `ConCommand` class with callback function support
- [ ] `FindVar()` function to locate variables by name
- [ ] `SetValue()` and `GetInt()`, `GetFloat()`, `GetString()` methods
- [ ] Cvar registration system on construction
- [ ] Flag system with bitwise operations
- [ ] Support for min/max range validation
- [ ] Callback function support for value changes
- [ ] Persistent storage (config.cfg parsing)
- [ ] Console input parsing for variable setting syntax
- [ ] Help text system for variables and commands
- [ ] Support for default value restoration via `Revert()`

## Integration Points

The console system should be callable from:
1. **User input**: Parse commands typed in console UI
2. **Configuration files**: Load autoexec.cfg, config.cfg, server.cfg
3. **Command-line arguments**: Process `+cvar_name value` startup args
4. **Game code**: Direct C++ access to ConVar objects
5. **Networking**: Replicated variables sync between server/client

## Key Insights for Implementation

1. **Naming is API**: Variable names (especially `cl_` and `sv_` prefixes) define behavior scope
2. **Flags are enforcement**: FCVAR flags determine access control and persistence
3. **String-first design**: Internally everything is strings, typed access is conversion layer
4. **Immutability during gameplay**: Certain flags should make variables read-only without cheats enabled
5. **Callback chain**: Changes should trigger notifications before actual value change
6. **Synchronization**: Replicated variables need network transport layer
