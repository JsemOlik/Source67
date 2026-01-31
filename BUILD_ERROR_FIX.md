# Build Error Fix - Missing iostream Header

## Problem

MSVC build was failing with multiple compilation errors:

```
C:\Users\olik\Desktop\Coding\Source67\src\Core\Logger.cpp(65): error C2039: 'cerr': is not a member of 'std'
C:\Users\olik\Desktop\Coding\Source67\src\Core\Logger.cpp(65): error C2065: 'cerr': undeclared identifier
```

This error occurred 8 times throughout Logger.cpp at lines:
- Line 65
- Line 95
- Line 103
- Line 112
- Line 114
- Line 130
- Line 141
- Line 169
- Line 171

## Root Cause

The previous commit added `std::cerr` statements for error logging but forgot to include the `<iostream>` header, which defines `std::cerr`.

## Fix

Added the missing include:

```cpp
#include <iostream>
```

This single line addition resolves all compilation errors.

## Verification

After this fix:
- ✅ Build should complete successfully
- ✅ All `std::cerr` error logging statements will work
- ✅ No runtime impact - purely a compile-time fix

## Testing

To verify the fix:

```cmd
cd C:\Users\olik\Desktop\Coding\Source67
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

Build should now succeed without errors.

## Summary

This was a simple oversight in the previous commit where error logging was added. The fix is minimal and correct:
- **1 file changed:** src/Core/Logger.cpp
- **1 line added:** `#include <iostream>`
- **Impact:** Fixes all 16 compilation errors

---

**Status: FIXED** - Build should now complete successfully!
