# Developer Console Quick Reference

## ConVar Quick Start

### Declare a Variable
```cpp
#include <convar.h>

// Basic variable
ConVar sv_gravity("sv_gravity", "800", FCVAR_ARCHIVE, "Gravity strength");

// With callback
void OnGravityChange(IConVar *var, const char *old, float flOld) {
    Msg("Gravity changed to %f\n", ((ConVar*)var)->GetFloat());
}
ConVar sv_gravity("sv_gravity", "800", FCVAR_ARCHIVE, 
    "Gravity strength", OnGravityChange);

// With range validation
ConVar cl_cmdrate("cl_cmdrate", "30", 0, "Command rate",
    true, 1.0f,        // min enabled, min value
    true, 128.0f       // max enabled, max value
);

// Static (file-scope only)
static ConVar my_var("my_var", "42", FCVAR_ARCHIVE, "My variable");
```

### Access Variables
```cpp
// Get values
int intVal = my_var.GetInt();
float floatVal = my_var.GetFloat();
const char *strVal = my_var.GetString();
bool boolVal = (my_var.GetInt() != 0);

// Set values
my_var.SetValue(42);
my_var.SetValue(42.5f);
my_var.SetValue("42");

// Metadata
const char *name = my_var.GetName();        // "my_var"
const char *help = my_var.GetHelpString();  // Help text

// Reset to default
my_var.Revert();

// Check if flag is set
if (my_var.IsFlagSet(FCVAR_CHEAT)) { }

// Add flags after creation
my_var.AddFlags(FCVAR_NOTIFY);
```

### Find Variables from Code
```cpp
// Lookup by name
ConVar *pGrav = cvar->FindVar("sv_gravity");
if (pGrav) {
    float gravity = pGrav->GetFloat();
}

// Cache pointer for repeated use
static ConVar *pGravity = nullptr;
if (!pGravity) {
    pGravity = cvar->FindVar("sv_gravity");
}
if (pGravity) {
    // Use pGravity
}
```

## ConCommand Quick Start

### Declare a Command
```cpp
#include <convar.h>

// Simple command (no arguments)
void MyCommand_f(const CCommand &args) {
    Msg("My command was called\n");
}
ConCommand my_command("my_command", MyCommand_f, "Does something");

// Command with arguments
void Say_f(const CCommand &args) {
    if (args.ArgC() < 2) {
        Msg("Usage: say <message>\n");
        return;
    }
    const char *message = args.Arg(1);
    Msg("You said: %s\n", message);
}
ConCommand say("say", Say_f, "Say something");

// Command with flags
void God_f(const CCommand &args) {
    if (args.ArgC() < 2) {
        Msg("Usage: god <0|1>\n");
        return;
    }
    player->SetGodMode(atoi(args.Arg(1)));
}
ConCommand god("god", God_f, "God mode", FCVAR_CHEAT);

// With auto-completion
static int SayAutoComplete(const char *partial,
    char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
    strcpy(commands[0], "hello");
    strcpy(commands[1], "goodbye");
    return 2;
}
ConCommand say("say", Say_f, "Say something", 0, SayAutoComplete);
```

## Cvar Naming Patterns

### Client Variables (cl_)
| Variable | Type | Range | Default | Purpose |
|----------|------|-------|---------|---------|
| `cl_cmdrate` | int | 1-128 | 30 | Packets/sec to server |
| `cl_updaterate` | int | 1-128 | 20 | Updates/sec from server |
| `cl_smooth` | bool | 0-1 | 1 | Smooth prediction errors |
| `cl_smoothtime` | float | 0+ | 0 | Smooth duration (seconds) |
| `cl_backspeed` | float | 0+ | 400 | Backward movement speed |
| `cl_forwardspeed` | float | 0+ | 450 | Forward movement speed |
| `cl_sidespeed` | float | 0+ | 450 | Strafe movement speed |
| `cl_drawparticles` | bool | 0-1 | 1 | Show particles |
| `cl_bobup` | float | 0-1 | 0.5 | View bob while moving |
| `cl_autowepswitch` | bool | 0-1 | 1 | Auto-switch on pickup |
| `name` | string | | "Player" | Player name (USERINFO) |

### Server Variables (sv_)
| Variable | Type | Range | Default | Purpose |
|----------|------|-------|---------|---------|
| `sv_gravity` | float | 0+ | 800 | World gravity |
| `sv_acceleration` | float | 0+ | 5.5 | Movement acceleration |
| `sv_airaccelerate` | float | 0+ | 12 | Air strafing accel |
| `sv_friction` | float | 0+ | 5.2 | Ground friction |
| `sv_cheats` | bool | 0-1 | 0 | Enable cheat commands |
| `sv_maxrate` | int | 0+ | unlimited | Max bandwidth per client |
| `sv_maxplayers` | int | 1-64 | 32 | Server player limit |
| `sv_tickrate` | int | 1+ | 64 | Server tick rate |
| `sv_timescale` | float | 0.1+ | 1.0 | Game speed (0.5=slow, 2=fast) |
| `sv_password` | string | | "" | Server password |
| `hostname` | string | | "Server" | Server name |

### Gameplay Rules (mp_)
| Variable | Type | Range | Default | Purpose |
|----------|------|-------|---------|---------|
| `mp_friendlyfire` | bool | 0-1 | 0 | Enable friendly fire |
| `mp_roundtime` | float | 0+ | 5 | Round duration (min) |
| `mp_timelimit` | float | 0+ | 0 | Map time limit (min) |
| `mp_damagescale` | float | 0+ | 1.0 | Damage multiplier |
| `mp_startmoney` | int | 0+ | 800 | Starting player money |

### Rendering Variables (r_)
| Variable | Type | Range | Default | Purpose |
|----------|------|-------|---------|---------|
| `r_drawparticles` | bool | 0-1 | 1 | Show particles |
| `r_shadows` | bool | 0-1 | 1 | Dynamic shadows |
| `r_skybox` | bool | 0-1 | 1 | Show skybox |
| `r_dynamic` | bool | 0-1 | 1 | Dynamic lighting |
| `r_drawdecals` | bool | 0-1 | 1 | Show decals |

## FCVAR Flags Quick Reference

| Flag | Effect | Example |
|------|--------|---------|
| `FCVAR_CHEAT` | Requires `sv_cheats 1` | god, noclip, spawn items |
| `FCVAR_ARCHIVE` | Saved to config.cfg | cl_cmdrate, sensitivity |
| `FCVAR_REPLICATED` | Server → all clients | sv_gravity, mp_friendlyfire |
| `FCVAR_USERINFO` | Client → server | name, cl_team |
| `FCVAR_NOTIFY` | Announce in chat | mp_friendlyfire change |
| `FCVAR_SERVER_CAN_EXECUTE` | Server can run on client | Remote config commands |
| `FCVAR_PROTECTED` | Server can't query | Password, private settings |
| `FCVAR_NEVER_AS_STRING` | Force typed access | Performance optimization |
| `FCVAR_RELEASE` | Release builds only | Debug variables hidden |
| `FCVAR_DONTRECORD` | Not in demos | Temporary settings |

### Common Flag Combinations
```cpp
// User preference (persists and archives)
ConVar volume("volume", "0.5", FCVAR_ARCHIVE, "Master volume");

// Player info (sent to server, persists)
ConVar name("name", "Player", FCVAR_ARCHIVE | FCVAR_USERINFO, "Player name");

// Game rule (replicated, announced, archived)
ConVar mp_ff("mp_friendlyfire", "0", 
    FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_ARCHIVE, "Friendly fire");

// Cheat (locked without sv_cheats)
ConVar god_mode("god", "0", FCVAR_CHEAT, "God mode");

// Client-only graphics (archived, no sync needed)
ConVar shadows("r_shadows", "1", FCVAR_ARCHIVE, "Dynamic shadows");

// Debug variable (cheat, won't record in demos)
ConVar showtriggers("showtriggers", "0", FCVAR_CHEAT | FCVAR_DONTRECORD, "...");
```

## Console Command Examples

### User Input (Console)
```
sv_gravity 400              // Change gravity to 400
cl_cmdrate 128              // Set command rate to 128
mp_friendlyfire 1           // Enable friendly fire
name "MyName"               // Set player name
say Hello World             // Say message
god 1                       // Enable god mode (if sv_cheats 1)
noclip                      // No collision (if sv_cheats 1)
changelevel de_dust2        // Change map
exec autoexec.cfg           // Load config file
```

### Config File (autoexec.cfg)
```cfg
// Client settings - persisted
cl_cmdrate 128
cl_updaterate 128
sensitivity 2.5
name "MyName"
bind w +forward
bind s +back
bind a +moveleft
bind d +moveright

// Volume and graphics
volume 0.7
r_shadows 1
r_drawparticles 1
```

### Server Config (server.cfg)
```cfg
// Server info
hostname "My Server"
sv_password ""
sv_maxplayers 32

// Game rules
sv_gravity 800
mp_friendlyfire 0
mp_roundtime 5
mp_timelimit 0

// Network
sv_maxrate 0
sv_minrate 20000
sv_tickrate 64

// No cheats in online play
sv_cheats 0
sv_pure 2
```

## Type Conversion Reference

### ConVar Type Access
```cpp
ConVar my_var("my_var", "42", FCVAR_ARCHIVE, "");

// All of these work with value "42":
my_var.GetInt()      // → 42
my_var.GetFloat()    // → 42.0f
my_var.GetString()   // → "42"
(my_var.GetInt() != 0)  // → true (boolean)

// String values:
ConVar str_var("str_var", "hello", FCVAR_ARCHIVE, "");
str_var.GetString()  // → "hello"
str_var.GetInt()     // → 0 (failed parse)
str_var.GetFloat()   // → 0.0f (failed parse)

// Boolean representation:
// False: 0, 0.0, "", "0", "false"
// True: any other value (usually 1 or 1.0)
```

## Debugging Commands

```
// List all variables
cvarlist                    // All cvars
cvarlist sv_                // All starting with "sv_"
cvarlist cl_ 10             // First 10 starting with "cl_"

// Search
find sv_gravity             // Find "sv_gravity"
findflags FCVAR_CHEAT       // Find all cheat cvars

// View single variable
sv_gravity                  // Print current value
help sv_gravity             // Print with help text

// Modify and test
sv_gravity 400              // Change value
sv_gravity                  // Verify change

// List all commands
cmdlist                     // All commands
cmdlist say                 // Find "say" command
```

## Common Implementation Tasks

### Task: Make variable archive-able (persist across sessions)
```cpp
ConVar my_setting("my_setting", "42", FCVAR_ARCHIVE, "My setting");
// Automatically saved to config.cfg on shutdown
// Automatically loaded on startup
```

### Task: Sync variable to all clients
```cpp
ConVar mp_rule("mp_friendlyfire", "0", 
    FCVAR_REPLICATED | FCVAR_NOTIFY, "...");
// Server changes → all clients forced to match
// Notifies clients via chat message
```

### Task: Send client setting to server
```cpp
ConVar name("name", "Player", FCVAR_USERINFO, "Player name");
// When client changes → server is notified
// Server can query via GetClientConVarValue()
```

### Task: Restrict to cheats mode
```cpp
ConVar god_mode("god", "0", FCVAR_CHEAT, "God mode");
// In multiplayer: only works if sv_cheats 1
// In singleplayer: no restriction (check sv_cheats in code if needed)
```

### Task: Validate input range
```cpp
ConVar cl_rate("cl_rate", "30", 0, "Command rate",
    true, 1.0f,      // Min: 1
    true, 128.0f     // Max: 128
);
// User enters 200 → clamped to 128
// User enters 0 → clamped to 1
```

### Task: React to variable changes
```cpp
void OnPhysicsVarChange(IConVar *var, const char *oldVal, float flOldVal) {
    // var->GetName() = name of changed variable
    // oldVal = old string value
    // flOldVal = old float value
    // var->GetInt/Float/String() = new value
}

ConVar sv_gravity("sv_gravity", "800", FCVAR_ARCHIVE, 
    "Gravity", OnPhysicsVarChange);
```

### Task: Protect from server queries
```cpp
ConVar password_hint("password_hint", "", 
    FCVAR_ARCHIVE | FCVAR_PROTECTED, "...");
// Server cannot query this value from clients
// Client can see it locally
```

### Task: Create checksum-safe variable
```cpp
ConVar integrity_var("integrity_var", "42", 
    FCVAR_DONTRECORD, "...");
// Won't be part of demo checksum
// Useful for local settings that don't affect gameplay
```

## Source Code Structure

**Where to find the Source Engine console code:**
```
src/
├── public/
│   ├── convar.h              // ConVar and ConCommand classes
│   └── tier1/
│       └── iconvar.h         // ICvar interface and FCVAR flags
├── engine/
│   ├── convar.cpp            // Console variable implementation
│   └── cmd.cpp               // Command execution
└── game/
    └── server/
        └── cbase.cpp         // Example cvar usage in game code
```

**Key files to include:**
```cpp
#include <convar.h>            // ConVar, ConCommand, CCommand
#include <tier0/dbg.h>         // Msg(), DevMsg(), Warning()
```

## Performance Notes

1. **Cvar lookup is O(n)** - iterate through linked list
   - Solution: Cache `ConVar*` pointers
   
2. **Type conversion on every access** - string→int/float conversion
   - Solution: Cache converted values locally if used frequently
   
3. **Callback overhead per change** - may trigger physics updates
   - Solution: Batch changes, avoid per-frame modifications
   
4. **Network sync cost** - replicated vars use bandwidth
   - Solution: Minimize change frequency, group changes
   
5. **Config file parsing** - linear execution of console commands
   - Solution: Minimize config file size, load in background

## Edge Cases & Gotchas

### Gotcha: FCVAR_ARCHIVE + FCVAR_CHEAT
```cpp
// BAD - user settings get reset when sv_cheats disabled
ConVar test("test", "0", FCVAR_ARCHIVE | FCVAR_CHEAT, "...");

// GOOD - separate archive and cheat settings
ConVar test("test", "0", FCVAR_ARCHIVE, "User setting");
ConVar test_cheat("test_cheat", "0", FCVAR_CHEAT, "Cheat version");
```

### Gotcha: Type Mismatches
```cpp
// String value with integer getter
ConVar bad("bad", "not_a_number", 0, "");
int val = bad.GetInt();  // Returns 0, silently!

// Solution: Document expected types
ConVar good("good", "42", 0, "Integer value (0-100)");
```

### Gotcha: Callback Can Create Infinite Loop
```cpp
// BAD - callback changes the same variable
void BadCallback(IConVar *v, const char *old, float flOld) {
    ((ConVar*)v)->SetValue(999);  // Triggers callback again!
}

// GOOD - set different variable or use flag
void GoodCallback(IConVar *v, const char *old, float flOld) {
    ConVar *other = cvar->FindVar("other_var");
    other->SetValue(999);  // Different variable, no recursion
}
```

### Gotcha: Replicated Variable Conflicts
```cpp
// Client can't change replicated var - server forces it
ConVar shared("shared", "0", FCVAR_REPLICATED, "...");

// In client code:
shared.SetValue(1);  // No effect! Server value overrides

// Solution: Client-specific var for display, replicated for sync
ConVar shared("shared", "0", FCVAR_REPLICATED, "Server value");
ConVar local_copy("local_copy", "0", 0, "Client cache");  // Updated by callback
```
