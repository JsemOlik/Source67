# Code Review: Application.h and Application.cpp

## Executive Summary
This review analyzes the Application class for memory leaks, resource management, thread safety, RAII patterns, error handling, OpenGL context management, ImGui integration, and physics system integration.

**Overall Assessment**: The code is generally well-structured with good use of smart pointers, but has several critical issues that need attention.

---

## Critical Issues (Must Fix)

### 1. **Memory Leak: Physics Bodies Not Properly Cleaned Up**
**Severity**: 🔴 CRITICAL  
**Location**: Application.cpp:267-270 (Destructor)  
**Issue**: The destructor calls `PhysicsSystem::Shutdown()` but does NOT remove/destroy individual physics bodies before shutdown. Scene entities contain `JPH::BodyID` references that should be cleaned up first.

```cpp
Application::~Application() {
  m_ImGuiLayer->OnDetach();
  PhysicsSystem::Shutdown();
}
```

**Problem**: 
- Scene entities hold `PhysicsBody` IDs that are not cleaned up
- `m_Scene->Clear()` is never called before shutdown
- Physics bodies may leak if the PhysicsSystem doesn't handle orphaned IDs

**Fix**:
```cpp
Application::~Application() {
  // Stop any running simulation
  if (m_SceneState != SceneState::Edit) {
    OnSceneStop();
  }
  
  // Clean up physics bodies before shutting down physics system
  if (m_Scene) {
    auto &bodyInterface = PhysicsSystem::GetBodyInterface();
    for (auto &entity : m_Scene->GetEntities()) {
      if (!entity->PhysicsBody.IsInvalid()) {
        bodyInterface.RemoveBody(entity->PhysicsBody);
        bodyInterface.DestroyBody(entity->PhysicsBody);
        entity->PhysicsBody = JPH::BodyID();
      }
    }
    m_Scene->Clear();
  }
  
  m_ImGuiLayer->OnDetach();
  PhysicsSystem::Shutdown();
}
```

---

### 2. **OpenGL State Corruption: Missing State Restoration**
**Severity**: 🔴 CRITICAL  
**Location**: Application.cpp:1731-1764 (RenderFrame)  
**Issue**: OpenGL state changes (stencil test, depth test, polygon mode, line width) are not properly wrapped with state save/restore, and some cleanup happens conditionally.

```cpp
if (entity == selectedEntity) {
  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 1, 0xFF);
  glStencilMask(0xFF);
}
// ... rendering ...
if (entity == selectedEntity)
  glStencilMask(0x00);  // Only set if entity == selectedEntity
```

Later:
```cpp
if (selectedEntity) {
  // ... more state changes ...
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // ...
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glStencilMask(0xFF);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
}
```

**Problems**:
1. If `selectedEntity` becomes null between frames, stencil state may persist
2. No RAII wrapper for GL state
3. Line width is set but never restored to default
4. Stencil mask manipulation is scattered

**Fix**: Create RAII state guards or ensure cleanup always happens:
```cpp
// In RenderFrame, before entity loop:
struct GLStateGuard {
  GLint oldStencilFunc, oldStencilRef, oldStencilMask;
  GLboolean oldStencilTest, oldDepthTest;
  GLint oldPolygonMode[2];
  GLfloat oldLineWidth;
  
  GLStateGuard() {
    glGetIntegerv(GL_STENCIL_FUNC, &oldStencilFunc);
    glGetIntegerv(GL_STENCIL_REF, &oldStencilRef);
    glGetIntegerv(GL_STENCIL_VALUE_MASK, &oldStencilMask);
    oldStencilTest = glIsEnabled(GL_STENCIL_TEST);
    oldDepthTest = glIsEnabled(GL_DEPTH_TEST);
    glGetIntegerv(GL_POLYGON_MODE, oldPolygonMode);
    glGetFloatv(GL_LINE_WIDTH, &oldLineWidth);
  }
  
  ~GLStateGuard() {
    if (!oldStencilTest) glDisable(GL_STENCIL_TEST);
    if (!oldDepthTest) glDisable(GL_DEPTH_TEST);
    glStencilMask(0xFF);
    glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode[0]);
    glLineWidth(oldLineWidth);
  }
};

// Use: GLStateGuard guard; at start of render pass
```

---

### 3. **Race Condition: Static SceneBackup Access**
**Severity**: 🟡 MEDIUM  
**Location**: Application.cpp:46-52, 282-360  
**Issue**: Global static `s_SceneBackup` is accessed without synchronization. If Application ever becomes multi-threaded (e.g., async asset loading), this will race.

```cpp
struct SceneBackup {
  struct TransformData {
    glm::vec3 Position, Rotation, Scale;
  };
  std::unordered_map<void *, TransformData> Data;
};
static SceneBackup s_SceneBackup;  // GLOBAL STATIC - NOT THREAD SAFE
```

**Fix**: Make it a member variable instead of static:
```cpp
// In Application.h:
struct SceneBackup {
  struct TransformData {
    glm::vec3 Position, Rotation, Scale;
  };
  std::unordered_map<void *, TransformData> Data;
};
SceneBackup m_SceneBackup;  // Member, not static

// In Application.cpp, change all s_SceneBackup to m_SceneBackup
```

**Additional Risk**: Using `void*` as key (entity.get()) is dangerous if entities are reallocated. Consider using entity IDs or UUIDs instead.

---

### 4. **Resource Leak: Potential Shader/Texture Leak on Reload**
**Severity**: 🟡 MEDIUM  
**Location**: Application.cpp:713-764 (OpenScene)  
**Issue**: When loading a scene, entities may have shaders/textures that are replaced without cleanup. The SceneSerializer may create new shader/texture instances.

```cpp
void Application::OpenScene(const std::string &filepath) {
  PhysicsSystem::Shutdown(); // Reset physics system to clear all bodies
  PhysicsSystem::Init();
  m_PlayerController = CreateScope<PlayerController>(m_Camera);

  DiscoverProject(std::filesystem::path(filepath));
  SceneSerializer serializer(m_Scene.get(), m_ProjectRoot.string());
  if (serializer.Deserialize(filepath)) {
    // Entities now have new Mesh/Shader/Texture references
    // Old references (if any) are dropped
    // If they're Ref<>, they auto-delete, but worth verifying
```

**Analysis Needed**: Check if `Entity` class properly uses `Ref<Shader>`, `Ref<Texture2D>`, `Ref<VertexArray>`. If yes, this is OK. If using raw pointers, this leaks.

**Recommendation**: Verify Entity class uses smart pointers. Add assertion:
```cpp
static_assert(std::is_same_v<decltype(Entity::Mesh), Ref<VertexArray>>, 
              "Entity::Mesh must be Ref<VertexArray>");
```

---

### 5. **Dangerous Pointer Usage: Entity as UserData**
**Severity**: 🟡 MEDIUM  
**Location**: Application.cpp:238, 259, 758, 789  
**Issue**: Raw entity pointer stored as physics body user data. If entity is deleted, physics system holds dangling pointer.

```cpp
floorSettings.mUserData = (uint64_t)floor.get();  // RAW POINTER
```

**Problems**:
- If entity is deleted while physics body exists, dangling pointer
- No lifetime validation when retrieving user data
- Raycasting retrieves this pointer without validation (line 846-853)

**Fix Options**:
1. Store entity UUID/ID instead of pointer
2. Use weak_ptr (requires Entity to be shared_from_this)
3. Add validation before dereferencing:

```cpp
// When creating body:
settings.mUserData = entity->GetUUID();  // Or entity->GetID()

// When raycasting:
JPH::BodyID hitID = PhysicsSystem::Raycast(rayOrigin, rayDirection, 1000.0f);
if (!hitID.IsInvalid()) {
  uint64_t userData = bodyInterface.GetUserData(hitID);
  for (auto &entity : m_Scene->GetEntities()) {
    if (entity->GetUUID() == userData) {
      m_SceneHierarchyPanel->SetSelectedEntity(entity);
      break;
    }
  }
}
```

---

## High Priority Issues

### 6. **Missing Error Handling: File Operations**
**Severity**: 🟠 HIGH  
**Location**: Multiple locations  
**Examples**:
- Application.cpp:462-471 (SaveManifest)
- Application.cpp:528-538 (DiscoverProject manifest parsing)

```cpp
std::ofstream fout(manifestPath);
if (fout.is_open()) {
  fout << "ProjectName: " << m_ProjectName << "\n";
  fout << "Version: " << m_ProjectVersion << "\n";
  fout.close();
  // NO CHECK for write errors!
}
```

**Fix**: Check stream state:
```cpp
std::ofstream fout(manifestPath);
if (!fout.is_open()) {
  S67_CORE_ERROR("Failed to open manifest for writing: {0}", manifestPath.string());
  return;
}

fout << "ProjectName: " << m_ProjectName << "\n";
fout << "Version: " << m_ProjectVersion << "\n";

if (!fout.good()) {
  S67_CORE_ERROR("Failed to write manifest data to {0}", manifestPath.string());
  return;
}

fout.close();
if (fout.fail()) {
  S67_CORE_ERROR("Failed to close manifest file: {0}", manifestPath.string());
  return;
}

S67_CORE_INFO("Saved project manifest to {0}", manifestPath.string());
```

---

### 7. **ImGui Context Management: No Verification**
**Severity**: 🟠 HIGH  
**Location**: Application.cpp:120-121, 268  
**Issue**: ImGui layer is attached in constructor and detached in destructor, but no verification that ImGui context is valid.

```cpp
m_ImGuiLayer = CreateScope<ImGuiLayer>();
m_ImGuiLayer->OnAttach();
```

**Potential Issue**: If ImGuiLayer::OnAttach() fails silently, all subsequent ImGui calls will crash or behave incorrectly.

**Fix**:
```cpp
m_ImGuiLayer = CreateScope<ImGuiLayer>();
if (!m_ImGuiLayer->OnAttach()) {
  S67_CORE_ERROR("Failed to initialize ImGui layer!");
  throw std::runtime_error("ImGui initialization failed");
}

// In destructor:
if (m_ImGuiLayer) {
  m_ImGuiLayer->OnDetach();
}
```

**Also Check**: ImGuiLayer::OnAttach() should return bool to indicate success/failure.

---

### 8. **Framebuffer Resize: No Validation**
**Severity**: 🟠 HIGH  
**Location**: Application.cpp:1633-1650  
**Issue**: Framebuffer resize calls don't check for allocation failures or invalid sizes.

```cpp
if (FramebufferSpecification spec = m_SceneFramebuffer->GetSpecification();
    m_SceneViewportSize.x > 0.0f && m_SceneViewportSize.y > 0.0f &&
    (spec.Width != (uint32_t)m_SceneViewportSize.x ||
     spec.Height != (uint32_t)m_SceneViewportSize.y)) {
  m_SceneFramebuffer->Resize((uint32_t)m_SceneViewportSize.x,
                             (uint32_t)m_SceneViewportSize.y);
  // NO ERROR CHECK
}
```

**Fix**: Add size limits and error checking:
```cpp
// Define max texture size (query from GL or use conservative value)
constexpr uint32_t MAX_FRAMEBUFFER_SIZE = 16384;

if (m_SceneViewportSize.x > 0.0f && m_SceneViewportSize.y > 0.0f) {
  uint32_t newWidth = static_cast<uint32_t>(m_SceneViewportSize.x);
  uint32_t newHeight = static_cast<uint32_t>(m_SceneViewportSize.y);
  
  // Clamp to reasonable limits
  newWidth = std::min(newWidth, MAX_FRAMEBUFFER_SIZE);
  newHeight = std::min(newHeight, MAX_FRAMEBUFFER_SIZE);
  
  if (spec.Width != newWidth || spec.Height != newHeight) {
    try {
      m_SceneFramebuffer->Resize(newWidth, newHeight);
    } catch (const std::exception& e) {
      S67_CORE_ERROR("Failed to resize scene framebuffer: {0}", e.what());
      // Fallback to previous size or minimum size
    }
    
    m_EditorCamera->SetProjection(
        m_EditorFOV, static_cast<float>(newWidth) / static_cast<float>(newHeight), 
        0.1f, 100.0f);
  }
}
```

---

## Medium Priority Issues

### 9. **RAII Violation: Manual Cursor Lock Management**
**Severity**: 🟡 MEDIUM  
**Location**: Multiple locations  
**Issue**: Cursor locking/unlocking is manual and error-prone. If an exception occurs between lock and unlock, cursor stays locked.

```cpp
m_Window->SetCursorLocked(true);
m_CursorLocked = true;
// ... code that might throw ...
m_Window->SetCursorLocked(false);
m_CursorLocked = false;
```

**Fix**: Create RAII guard:
```cpp
class CursorLockGuard {
  Window* m_Window;
  bool m_WasLocked;
  bool* m_LockFlag;
public:
  CursorLockGuard(Window* window, bool* lockFlag, bool shouldLock) 
    : m_Window(window), m_LockFlag(lockFlag), m_WasLocked(*lockFlag) {
    if (shouldLock && !m_WasLocked) {
      m_Window->SetCursorLocked(true);
      *m_LockFlag = true;
    }
  }
  
  ~CursorLockGuard() {
    if (*m_LockFlag != m_WasLocked) {
      m_Window->SetCursorLocked(m_WasLocked);
      *m_LockFlag = m_WasLocked;
    }
  }
};

// Usage:
void Application::OnScenePlay() {
  // ... setup ...
  CursorLockGuard guard(m_Window.get(), &m_CursorLocked, true);
  // Cursor automatically unlocked on scope exit if exception occurs
}
```

---

### 10. **Spin-Wait CPU Burn**
**Severity**: 🟡 MEDIUM  
**Location**: Application.cpp:978-983  
**Issue**: Active spin-wait burns CPU cycles unnecessarily.

```cpp
if (minFrameTime > 0.0f && elapsed < minFrameTime) {
  while ((float)glfwGetTime() - m_LastFrameTime < minFrameTime) {
    // Spin wait  <-- 100% CPU usage
  }
}
```

**Fix**: Use sleep for better CPU efficiency:
```cpp
if (minFrameTime > 0.0f && elapsed < minFrameTime) {
  float remaining = minFrameTime - elapsed;
  
  // Sleep for most of the time (leave 1ms for precision)
  if (remaining > 0.002f) {
    auto sleepTime = std::chrono::duration<float>(remaining - 0.001f);
    std::this_thread::sleep_for(sleepTime);
  }
  
  // Spin for the last bit for precision
  while ((float)glfwGetTime() - m_LastFrameTime < minFrameTime) {
    std::this_thread::yield();
  }
}
```

---

### 11. **Inconsistent Scene State Management**
**Severity**: 🟡 MEDIUM  
**Location**: Application.cpp:272-361 (OnScenePlay/Pause/Stop)  
**Issue**: State transitions don't validate current state properly.

```cpp
void Application::OnScenePause() {
  if (m_SceneState != SceneState::Play)
    return;  // Good - checks state
  
  m_SceneState = SceneState::Pause;
  // ... cleanup ...
}

void Application::OnScenePlay() {
  if (m_ProjectRoot.empty() || !m_LevelLoaded) {
    // ... error ...
    return;
  }
  
  // Missing: what if already in Play state?
  // Missing: what if in Pause state (should just resume, not backup again)
  
  if (m_SceneState == SceneState::Edit) {
    // Backup before first play
    s_SceneBackup.Data.clear();
    // ...
  }
}
```

**Fix**: Add proper state machine:
```cpp
void Application::OnScenePlay() {
  if (m_ProjectRoot.empty() || !m_LevelLoaded) {
    S67_CORE_WARN("Cannot enter Play Mode: No project or level loaded!");
    return;
  }
  
  switch (m_SceneState) {
    case SceneState::Play:
      // Already playing, nothing to do
      return;
      
    case SceneState::Pause:
      // Resume from pause
      m_SceneState = SceneState::Play;
      m_Window->SetCursorLocked(true);
      m_CursorLocked = true;
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
      return;
      
    case SceneState::Edit:
      // Fresh play - do full initialization
      m_Scene->EnsurePlayerExists();
      
      // Backup scene state
      m_SceneBackup.Data.clear();
      for (auto &entity : m_Scene->GetEntities()) {
        m_SceneBackup.Data[entity.get()] = {
          entity->Transform.Position,
          entity->Transform.Rotation,
          entity->Transform.Scale
        };
      }
      
      // ... rest of initialization ...
      m_SceneState = SceneState::Play;
      break;
  }
}
```

---

### 12. **Missing Const Correctness**
**Severity**: 🟢 LOW  
**Location**: Application.h:73-89  
**Issue**: Getter methods modify internal state or don't use const.

```cpp
inline Window &GetWindow() { return *m_Window; }  // Should return const Window& if not modifying
ImGuiLayer &GetImGuiLayer() { return *m_ImGuiLayer; }  // Should have const version
inline static Application &Get() { return *s_Instance; }  // OK for singleton
```

**Fix**: Add const versions where appropriate:
```cpp
const Window& GetWindow() const { return *m_Window; }
Window& GetWindow() { return *m_Window; }

const ImGuiLayer& GetImGuiLayer() const { return *m_ImGuiLayer; }
ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
```

---

## Thread Safety Analysis

### Current State: **NOT THREAD-SAFE**

**Identified Issues**:
1. ✅ **Single Instance**: `s_Instance` static - no mutex protection
2. ✅ **SceneBackup**: Static global - no synchronization
3. ✅ **Scene Access**: Direct access to `m_Scene` from multiple code paths
4. ✅ **Physics System**: Likely not thread-safe (Jolt needs thread-safe wrapper)
5. ✅ **OpenGL Context**: Must stay on main thread (correct as-is)

**Current Threading Model**: Single-threaded (main thread only) - this is CORRECT for OpenGL.

**If Future Multi-Threading is Needed**:
- Asset loading: Use separate thread with message queue
- Physics: Already thread-safe internally (Jolt), but BodyInterface access needs mutex
- Scene updates: Must stay on main thread (or use double-buffering)

---

## RAII Pattern Analysis

### Good RAII Usage ✅
1. `std::unique_ptr<Window> m_Window` (lines 103)
2. `Scope<ImGuiLayer> m_ImGuiLayer` (line 117)
3. `Scope<PlayerController>` (line 115)
4. `Scope<Scene>` (line 110)
5. Smart pointers (`Ref<>`, `Scope<>`) used throughout

### RAII Violations ❌
1. **OpenGL State Management**: No RAII guards (Issue #2)
2. **Cursor Locking**: Manual management (Issue #9)
3. **Physics Bodies**: Created but not explicitly destroyed in destructor (Issue #1)
4. **File Handles**: Using raw std::ofstream without validation (Issue #6)

---

## OpenGL Context Management

### Current Implementation
**Location**: Not visible in Application.cpp - likely in Window class

**Assumptions**:
- GLFW context created in Window::Create()
- Context remains current throughout application lifetime
- No context switching between threads

### Potential Issues
1. **No Context Validation**: Application assumes valid GL context exists
   ```cpp
   // Application.cpp:95
   Renderer::Init();  // Assumes GL context is current
   ```

2. **No Debug Context Verification**: 
   ```cpp
   // Recommended addition in Application constructor:
   GLint flags;
   glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
   if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
     S67_CORE_INFO("OpenGL debug context active");
   }
   
   // Enable debug output
   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
   glDebugMessageCallback(OpenGLMessageCallback, nullptr);
   ```

3. **No Framebuffer Completeness Checks**:
   ```cpp
   // After creating framebuffers (line 136-137):
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
     S67_CORE_ERROR("Scene framebuffer is not complete!");
   }
   ```

### Recommendations
1. Add GL context validation in constructor
2. Implement GL debug callback for KHR_debug
3. Check framebuffer completeness after creation/resize
4. Add GL error checking wrapper (debug builds)

---

## ImGui Integration Analysis

### Initialization
**Location**: Application.cpp:120-121  
**Status**: ✅ Good - using RAII with Scope<ImGuiLayer>

### Event Handling
**Location**: Application.cpp:800  
**Status**: ⚠️ Potential Issue

```cpp
void Application::OnEvent(Event &e) {
  m_ImGuiLayer->OnEvent(e);  // ImGui gets events first
  
  if (m_SceneState == SceneState::Play) {
    m_PlayerController->OnEvent(e);
  } else {
    // ... editor events ...
  }
}
```

**Issue**: ImGui doesn't block events when needed. Events may fall through to game/editor.

**Current Mitigation** (line 1991-1992):
```cpp
m_ImGuiLayer->SetBlockEvents(
    (!m_SceneViewportFocused || !m_SceneViewportHovered) && !ImGuizmo::IsOver());
```

**Status**: ✅ Correctly blocks events

### Frame Lifecycle
**Location**: Application.cpp:1791, 2350

```cpp
m_ImGuiLayer->Begin();  // Line 1791
// ... ImGui rendering ...
m_ImGuiLayer->End();    // Line 2350
```

**Status**: ✅ Correct pairing

### Potential Issues
1. **No ImGui Context Validity Check**: If ImGui fails to initialize, all ImGui calls will crash
2. **Docking/Viewport Features**: No verification these are enabled
3. **Font Loading**: No error handling if font fails to load

**Recommendations**:
```cpp
// After ImGuiLayer::OnAttach():
ImGuiIO& io = ImGui::GetIO();
if (!(io.ConfigFlags & ImGuiConfigFlags_DockingEnable)) {
  S67_CORE_WARN("ImGui docking not enabled!");
}

// When loading fonts:
if (!io.Fonts->Fonts.empty()) {
  S67_CORE_INFO("ImGui fonts loaded: {0}", io.Fonts->Fonts.Size);
} else {
  S67_CORE_ERROR("No ImGui fonts loaded - text will not render!");
}
```

---

## Physics System Integration

### Initialization/Shutdown
**Location**: Application.cpp:97-98, 269

```cpp
// Init:
PhysicsSystem::Init();

// Shutdown:
PhysicsSystem::Shutdown();
```

**Status**: ✅ Properly paired, but missing intermediate cleanup (see Issue #1)

### Body Management

#### Creation
**Examples**: Lines 233-240, 252-261, 759-760, 791-792

**Status**: ✅ Bodies created correctly with proper settings

**Good Practices**:
- User data set for entity association
- Layer separation (MOVING/NON_MOVING)
- Activation state controlled

#### Destruction
**Location**: Lines 772-773 (OnEntityCollidableChanged)

```cpp
bodyInterface.RemoveBody(entity->PhysicsBody);
bodyInterface.DestroyBody(entity->PhysicsBody);
```

**Status**: ✅ Proper two-step destruction (remove then destroy)

**Missing**: Destructor cleanup (Issue #1)

### Synchronization (Physics ↔ Rendering)

#### Edit Mode → Physics
**Location**: Lines 1720-1727

```cpp
glm::quat q = glm::quat(glm::radians(entity->Transform.Rotation));
bodyInterface.SetPositionAndRotation(
    entity->PhysicsBody,
    JPH::RVec3(entity->Transform.Position.x, entity->Transform.Position.y,
               entity->Transform.Position.z),
    JPH::Quat(q.x, q.y, q.z, q.w), 
    JPH::EActivation::DontActivate);
```

**Status**: ✅ Correctly syncs transform to physics in edit mode

#### Play Mode → Rendering
**Location**: Lines 1708-1718

```cpp
if (m_SceneState == SceneState::Play) {
  JPH::RVec3 position;
  JPH::Quat rotation;
  bodyInterface.GetPositionAndRotation(entity->PhysicsBody, position, rotation);
  entity->Transform.Position = {position.GetX(), position.GetY(), position.GetZ()};
  glm::quat q = {rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()};
  entity->Transform.Rotation = glm::degrees(glm::eulerAngles(q));
}
```

**Status**: ✅ Correctly syncs physics to transform in play mode

### Update Loop
**Location**: Lines 1652-1656

```cpp
if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause) {
  if (m_SceneState == SceneState::Play) {
    m_PlayerController->OnUpdate(timestep);
    PhysicsSystem::OnUpdate(timestep);
  }
}
```

**Status**: ⚠️ Potential Issue

**Problems**:
1. Timestep clamping not visible (may be in PhysicsSystem::OnUpdate)
2. No accumulator for fixed timestep (if needed)
3. Large timesteps (e.g., after debugger pause) may cause physics explosion

**Recommended Fix**:
```cpp
// Clamp timestep to prevent physics instability
constexpr float MAX_PHYSICS_TIMESTEP = 1.0f / 30.0f; // Max 30Hz (33ms)

if (m_SceneState == SceneState::Play) {
  float clampedTimestep = std::min(static_cast<float>(timestep), MAX_PHYSICS_TIMESTEP);
  m_PlayerController->OnUpdate(clampedTimestep);
  PhysicsSystem::OnUpdate(clampedTimestep);
}
```

### Raycasting
**Location**: Lines 845-856

```cpp
JPH::BodyID hitID = PhysicsSystem::Raycast(rayOrigin, rayDirection, 1000.0f);
if (!hitID.IsInvalid()) {
  for (auto &entity : m_Scene->GetEntities()) {
    if (entity->PhysicsBody == hitID) {
      m_SceneHierarchyPanel->SetSelectedEntity(entity);
      break;
    }
  }
}
```

**Status**: ⚠️ Inefficient

**Issues**:
1. O(n) search through all entities
2. Relies on pointer stored in UserData (Issue #5)

**Recommended Fix**: Use UserData for direct lookup (see Issue #5 fix)

---

## Error Handling Assessment

### Current State: **INADEQUATE**

**Good Error Handling** ✅:
1. Scene state validation (lines 273-276, 695-698)
2. File existence checks (lines 66-87, 481-494)
3. Try-catch in asset copying (lines 407-446)

**Missing Error Handling** ❌:
1. File I/O error checking (Issue #6)
2. OpenGL call validation (no glGetError checks)
3. Framebuffer completeness (Issue #8)
4. Memory allocation failures (assumes infinite memory)
5. Physics system failures (assumes Init() always succeeds)

### Recommendations

1. **Add GL Error Checking (Debug Builds)**:
```cpp
#ifdef S67_DEBUG
  #define GL_CHECK(call) do { \
    call; \
    GLenum err; \
    while ((err = glGetError()) != GL_NO_ERROR) { \
      S67_CORE_ERROR("OpenGL error {0} at {1}:{2} in {3}", \
                     err, __FILE__, __LINE__, #call); \
    } \
  } while(0)
#else
  #define GL_CHECK(call) call
#endif
```

2. **Validate Physics Initialization**:
```cpp
if (!PhysicsSystem::Init()) {
  S67_CORE_ERROR("Failed to initialize physics system!");
  throw std::runtime_error("Physics initialization failed");
}
```

3. **Add Resource Cleanup on Failure**:
```cpp
try {
  // ... initialization ...
} catch (const std::exception& e) {
  S67_CORE_ERROR("Application initialization failed: {0}", e.what());
  
  // Cleanup partial initialization
  if (m_ImGuiLayer) m_ImGuiLayer->OnDetach();
  PhysicsSystem::Shutdown();
  
  throw; // Re-throw after cleanup
}
```

---

## Performance Observations

### Positive Aspects ✅
1. Framebuffer resizing only when dimensions change (lines 1633-1650)
2. FPS capping to reduce unnecessary rendering (lines 972-986)
3. Early-out checks in state transitions

### Concerns ⚠️
1. **Spin-Wait CPU Burn** (Issue #10)
2. **Linear Search in Raycasting** (Issue #5 note)
3. **Excessive State Validation**: Same checks repeated in multiple places
4. **Framebuffer Binding Overhead**: Two full render passes every frame (Scene + Game views)

### Recommendations
1. Consider single render pass when Game view is hidden
2. Cache entity lookups by physics BodyID
3. Profile ImGui rendering (often the bottleneck in editor apps)

---

## Summary of Recommendations

### Must Fix (Before Release)
1. ✅ Fix destructor to clean up physics bodies (Issue #1)
2. ✅ Add RAII guards for OpenGL state (Issue #2)
3. ✅ Move static SceneBackup to member variable (Issue #3)
4. ✅ Add file I/O error handling (Issue #6)
5. ✅ Validate ImGui initialization (Issue #7)
6. ✅ Add framebuffer resize validation (Issue #8)

### Should Fix (High Priority)
7. ✅ Fix entity pointer usage in physics UserData (Issue #5)
8. ✅ Replace spin-wait with sleep (Issue #10)
9. ✅ Add proper state machine for scene states (Issue #11)
10. ✅ Add OpenGL debug context and error checking

### Nice to Have (Low Priority)
11. ✅ Add const correctness to getters (Issue #12)
12. ✅ Create RAII cursor lock guard (Issue #9)
13. ✅ Optimize raycasting with direct lookup
14. ✅ Add physics timestep clamping

---

## Code Quality Metrics

**Overall Grade**: B-

| Category | Grade | Notes |
|----------|-------|-------|
| Memory Management | B | Good use of smart pointers, but missing destructor cleanup |
| Resource Management | B- | Some leaks possible, file I/O needs work |
| Thread Safety | N/A | Single-threaded (correct for OpenGL) |
| RAII Patterns | B | Good overall, but missing GL state guards |
| Error Handling | C | Many paths lack error checking |
| OpenGL Management | B- | Works but needs validation and debug support |
| ImGui Integration | B+ | Well structured, minor improvements possible |
| Physics Integration | B+ | Good sync logic, but needs cleanup in destructor |
| Code Clarity | A- | Well organized and readable |
| Documentation | C | Minimal comments, complex logic undocumented |

---

## Final Notes

This is a solid codebase with good architecture. The main issues are:
1. **Cleanup paths incomplete** (destructor needs work)
2. **Error handling inconsistent** (add validation)
3. **RAII opportunities missed** (GL state, cursor management)

None of the issues are catastrophic, but they should be addressed before production use. The code follows modern C++ practices overall and makes good use of smart pointers and RAII in most places.

**Estimated Time to Fix Critical Issues**: 4-6 hours  
**Estimated Time to Fix All Issues**: 12-16 hours
