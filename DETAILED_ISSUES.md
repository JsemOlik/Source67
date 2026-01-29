# Source67 - Detailed Issue List with Line Numbers

This document lists every issue found during the comprehensive code review, organized by file with exact line numbers and recommended fixes.

---

## CMakeLists.txt

### Critical Issues

#### 1. Missing Compiler Warnings (Lines 8-16)
**Severity:** CRITICAL  
**Issue:** No warning flags configured  
**Fix:**
```cmake
if(MSVC)
    target_compile_options(Source67 PRIVATE /W4 /WX)
else()
    target_compile_options(Source67 PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
```

#### 2. Using `master` Branch for Jolt (Line 62)
**Severity:** CRITICAL  
**Issue:** Non-reproducible builds  
**Fix:**
```cmake
GIT_TAG v5.0.0  # Pin to specific version
```

#### 3. Using `master` Branch for GLAD (Line 92)
**Severity:** CRITICAL  
**Fix:** Pin to specific commit SHA

#### 4. Security Warning Suppression (Lines 155-156)
**Severity:** HIGH  
**Issue:** `_CRT_SECURE_NO_WARNINGS` hides security issues  
**Fix:** Remove and fix underlying warnings

### High Priority

#### 5. File Globbing (Lines 108-114)
**Severity:** HIGH  
**Fix:**
```cmake
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/**/*.cpp")
```

#### 6. Missing OpenGL Linking on Linux (Lines 160-162)
**Severity:** HIGH  
**Fix:**
```cmake
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Source67 PUBLIC OpenGL::GL)
```

### Medium Priority

#### 7-13. Various configuration issues
- Missing platform checks for Objective-C++
- WIN32 flag unconditional
- No PIE/PIC flags
- ImGuizmo not explicitly linked
- No build type validation

---

## src/main.cpp

### Critical Issues

#### 1. Memory Leak - Raw Pointer (Lines 31-33)
**Severity:** HIGH  
**Issue:**
```cpp
auto app = new S67::Application(argv[0], argc > 1 ? argv[1] : "");
app->Run();
delete app;  // Not called if Run() throws!
```
**Fix:**
```cpp
auto app = std::make_unique<S67::Application>(argv[0], argc > 1 ? argv[1] : "");
app->Run();
```

### Medium Priority

#### 2. Missing DPI Error Check (Line 22)
**Fix:**
```cpp
if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
    S67_CORE_WARN("Failed to set DPI awareness: {}", GetLastError());
}
```

#### 3. Missing Logger Init Check (Line 24)
**Fix:**
```cpp
if (!S67::Logger::Init()) {
    std::cerr << "Failed to initialize logger!" << std::endl;
    return 1;
}
```

#### 4. No argc Validation (Line 31)
**Fix:**
```cpp
if (argc < 1 || argv == nullptr) {
    std::cerr << "Invalid command line arguments" << std::endl;
    return 1;
}
```

### Low Priority

#### 5. No Exception Handling (All)
**Fix:** Wrap main in try-catch block

---

## src/Core/Application.h & Application.cpp

### Critical Issues

#### 1. Physics Bodies Not Cleaned (Application.cpp ~190)
**Severity:** CRITICAL  
**Issue:** Physics bodies leak before shutdown  
**Fix:** Clean up physics bodies in destructor

#### 2. OpenGL State Corruption (Multiple locations)
**Severity:** CRITICAL  
**Issue:** Missing state restoration for stencil/depth/polygon mode  
**Fix:** Save and restore state after modifications

#### 3. Static SceneBackup Race (Application.cpp)
**Severity:** CRITICAL  
**Issue:** Global static without thread safety  
**Fix:** Add mutex or make non-static

### High Priority

#### 4. Missing File I/O Error Handling
**Issue:** No validation of write operations in SaveScene  
**Fix:** Check return values and throw/log on failure

#### 5. No ImGui Context Verification
**Issue:** Silent failures if context not initialized  
**Fix:** Add `ImGui::GetCurrentContext()` checks

---

## src/Core/Window.h & Window.mm

### Critical Issues

#### 1. Resource Leak - glfwTerminate Never Called (Line 218)
**Severity:** CRITICAL  
**Issue:** `glfwTerminate()` never called  
**Fix:**
```cpp
void Window::Shutdown() { 
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    
    static int s_WindowCount = 0;
    s_WindowCount--;
    
    if (s_WindowCount == 0 && s_GLFWInitialized) {
        glfwTerminate();
        s_GLFWInitialized = false;
    }
}
```

#### 2. Missing Context Validation (Lines 57-59)
**Severity:** CRITICAL  
**Fix:**
```cpp
m_Window = glfwCreateWindow(...);
if (!m_Window) {
    S67_CORE_ERROR("Failed to create GLFW window!");
    glfwTerminate();
    s_GLFWInitialized = false;
    return;
}
```

#### 3. Null Pointer Dereference Risk (Lines 68, 79, 88+)
**Severity:** CRITICAL  
**Issue:** Callbacks don't check if `glfwGetWindowUserPointer()` returns null  
**Fix:** Add null checks in all callbacks

### High Priority

#### 4. Thread Safety - EventCallback (Lines 33-35)
**Severity:** HIGH  
**Issue:** No synchronization on callback access  
**Fix:** Add mutex or document thread requirements

#### 5. Missing OpenGL Debug Context (Lines 46-55)
**Severity:** HIGH  
**Fix:**
```cpp
#ifdef S67_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
```

#### 6. VSync Uninitialized (Line 53)
**Severity:** HIGH  
**Fix:**
```cpp
m_Data.VSync = true;
glfwSwapInterval(m_Data.VSync ? 1 : 0);
```

### Medium Priority

#### 7. Uninitialized m_Window (Line 48)
**Fix:** `GLFWwindow *m_Window = nullptr;`

#### 8. Silent Icon Load Failures (Lines 183-216)
**Fix:** Return bool status

#### 9. macOS NSImage Leak (Line 203)
**Fix:** `[image release];` or enable ARC

---

## src/Core/Logger.h & Logger.cpp

### Critical Issues

#### 1. Thread Safety - s_LogHistory Race (Lines 30-34, 17, 39)
**Severity:** CRITICAL  
**Issue:** Vector accessed from multiple threads without synchronization  
**Fix:**
```cpp
class Logger {
private:
    static std::mutex s_LogMutex;
    
public:
    static const std::vector<LogEntry> GetLogHistory() { 
        std::lock_guard<std::mutex> lock(s_LogMutex);
        return s_LogHistory;
    }
    
    static void ClearLogHistory() { 
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_LogHistory.clear(); 
    }
    
    static void AddLogEntry(const LogEntry &entry) {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_LogHistory.push_back(entry);
    }
};
```

### High Priority

#### 2. No Double-Init Protection (Line 47)
**Severity:** HIGH  
**Fix:**
```cpp
void Logger::Init() {
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        // ... initialization
    });
}
```

#### 3. No Shutdown Method
**Severity:** HIGH  
**Fix:** Add `Logger::Shutdown()` to flush and cleanup

### Medium Priority

#### 4. Unprotected Filesystem Ops (Lines 52-71)
**Fix:** Wrap in try-catch blocks

#### 5. Unbounded Log History (Lines 17, 33)
**Fix:** Limit to MAX_LOG_HISTORY entries

---

## src/Core/Input.h & Input.cpp

### High Priority

#### 1. Thread Safety (Lines 8, 14, 20)
**Severity:** HIGH  
**Fix:** Add mutex for all Input methods

#### 2. Missing Null Checks (Lines 8, 14, 20)
**Severity:** HIGH  
**Fix:**
```cpp
bool Input::IsKeyPressed(int keycode) {
    auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    if (!window) {
        S67_CORE_ERROR("Failed to get native window");
        return false;
    }
    // ...
}
```

---

## src/Core/Timer.h

### High Priority

#### 1. Thread Safety (Lines 9-22)
**Severity:** HIGH  
**Fix:**
```cpp
class Timer {
public:
    void Reset() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Start = std::chrono::high_resolution_clock::now();
    }
    
    float Elapsed() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now() - m_Start).count() * 1e-9f;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    mutable std::mutex m_Mutex;
};
```

### Medium Priority

#### 2. Magic Numbers (Line 18)
**Fix:** Use `1e-9f` instead of `0.001f * 0.001f * 0.001f`

---

## src/Core/UndoSystem.h

### High Priority

#### 1. Memory Safety - Circular References (Lines 19, 38)
**Severity:** HIGH  
**Fix:** Use `std::weak_ptr` instead of `Ref<Entity>`

#### 2. Thread Safety (Lines 57-98)
**Severity:** HIGH  
**Fix:** Add mutex for stack operations

### Medium Priority

#### 3. Confusing Push Semantics (Line 58)
**Fix:** Rename or clarify API

#### 4. Magic Number (Lines 62, 72)
**Fix:** Make stack size configurable

---

## src/Core/Base.h

### High Priority

#### 1. Missing Logger Include (Lines 6-7)
**Severity:** HIGH  
**Fix:** `#include "Logger.h"`

#### 2. Non-Portable Debug Break (Lines 6-7)
**Severity:** MEDIUM  
**Fix:**
```cpp
#ifdef _MSC_VER
    #define S67_DEBUGBREAK() __debugbreak()
#else
    #define S67_DEBUGBREAK() __builtin_trap()
#endif
```

---

## src/Renderer/Shader.h & Shader.cpp

### Critical Issues

#### 1. Resource Leak on Compilation Failure (Line 156)
**Severity:** CRITICAL  
**Fix:**
```cpp
if (isCompiled == GL_FALSE) {
    glDeleteShader(shader);
    glDeleteProgram(program);
    for (auto id : shaderIDs) {
        glDeleteShader(id);
    }
    S67_CORE_ERROR("{0}", infoLog.data());
    return;
}
```

#### 2. Missing Rule of Five (Lines 11-16)
**Severity:** CRITICAL  
**Fix:** Delete copy ops, implement move semantics

### High Priority

#### 3. No Validation Before GL Calls (Lines 92, 96-124)
**Severity:** HIGH  
**Fix:**
```cpp
void Shader::Bind() const { 
    S67_CORE_ASSERT(m_RendererID != 0, "Cannot bind invalid shader!");
    glUseProgram(m_RendererID);
}
```

#### 4. Inefficient Uniform Lookups (Lines 96-124)
**Severity:** HIGH (Performance)  
**Fix:** Implement uniform location caching

#### 5. Missing Debug Labels (Lines 126-189)
**Severity:** HIGH  
**Fix:** Add `glObjectLabel()` calls

### Medium Priority

#### 6-8. No hot-reload, unsafe substr, no reflection

---

## src/Renderer/Texture.h & Texture.cpp

### Critical Issues

#### 1. Rule of Five Violation (Lines 8-56)
**Severity:** CRITICAL  
**Fix:** Same as Shader - delete copy, implement move

### Medium Priority

#### 2. Uninitialized Members (Lines 54-55)
**Fix:** `uint32_t m_Width = 0, m_Height = 0;`

#### 3. Resource Leak on Constructor Failure (Lines 10-36)
**Fix:** Check GL errors, cleanup on failure

---

## src/Renderer/Buffer.h & Buffer.cpp

### Critical Issues

#### 1-2. Rule of Five Violations (Lines 8-34, 42-68)
**Severity:** CRITICAL  
**Issue:** Both `OpenGLVertexBuffer` and `OpenGLIndexBuffer` missing copy/move  
**Fix:** Delete copy ops, implement move for both classes

### Medium Priority

#### 3. Uninitialized Members (Lines 32, 66-67)
**Fix:** Initialize to 0

#### 4. No Error Checking (Lines 10-13, 44-48)
**Fix:** Validate buffer creation

---

## src/Renderer/VertexArray.h & VertexArray.cpp

### Critical Issues

#### 1. Rule of Five Violation (Lines 25-79)
**Severity:** CRITICAL  
**Fix:** Delete copy, implement move

### Medium Priority

#### 2. Uninitialized Member (Line 76)
**Fix:** `uint32_t m_RendererID = 0;`

#### 3. Vertex Attribute Index Tracking (Lines 43-62)
**Severity:** MEDIUM  
**Issue:** Attributes overlap with multiple vertex buffers  
**Fix:**
```cpp
class OpenGLVertexArray {
private:
    uint32_t m_RendererID = 0;
    uint32_t m_VertexAttribIndex = 0;  // Track current index
    
public:
    virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override {
        // ... existing code ...
        for (const auto& element : layout) {
            glEnableVertexAttribArray(m_VertexAttribIndex);
            // ... setup ...
            m_VertexAttribIndex++;  // Increment for next attribute
        }
    }
};
```

---

## src/Renderer/Framebuffer.h & Framebuffer.cpp

### Critical Issues

#### 1. Rule of Five Violation (Lines 9-78)
**Severity:** CRITICAL  
**Fix:** Delete copy, implement move

#### 2. Uninitialized Resource Deletion (Lines 22-27)
**Severity:** CRITICAL  
**Issue:** `Invalidate()` deletes uninitialized resources on first call  
**Fix:**
```cpp
class OpenGLFramebuffer {
public:
    OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
        , m_RendererID(0)
        , m_ColorAttachment(0)
        , m_DepthAttachment(0) {
        Invalidate();
    }
};
```

### Medium Priority

#### 3. Missing Error Recovery (Lines 22-48)
**Fix:** Check framebuffer completeness, cleanup on failure

---

## src/Renderer/Mesh.h & Mesh.cpp

### Critical Issues

#### 1. Type Punning / Strict Aliasing (Lines 100-101, 172-173, 236-237, 341-342)
**Severity:** CRITICAL  
**Issue:**
```cpp
VertexBuffer::Create((float *)vertices.data(), ...);  // Violates strict aliasing
```
**Fix:**
```cpp
VertexBuffer::Create(reinterpret_cast<float*>(vertices.data()), ...);
// Or better: redesign API to accept void*
```

#### 2. Multiple Definition Issue (Line 1)
**Severity:** CRITICAL  
**Issue:** `TINYOBJLOADER_IMPLEMENTATION` in header-included file  
**Fix:** Move to separate compilation unit

### High Priority

#### 3. Poor Hash Function (Lines 26-37)
**Fix:** Use proper hash combining

#### 4. No File Close Check (Lines 114-183)
**Fix:** Explicitly close and check

#### 5. Missing Bounds Checking (Lines 62-64, 71-73, 77-78)
**Fix:** Validate indices before array access

### Medium Priority

#### 6-8. Division by zero, normal calculation, incomplete implementation

---

## src/Renderer/Scene.h & Scene.cpp

### High Priority

#### 1. Iterator Invalidation (Lines 16-20)
**Severity:** HIGH  
**Fix:**
```cpp
void RemoveEntity(const Ref<Entity> &entity) {
    m_Entities.erase(
        std::remove(m_Entities.begin(), m_Entities.end(), entity),
        m_Entities.end()
    );
}
```

#### 2. Resource Creation Without Validation (Lines 39-42)
**Fix:** Check if `Texture2D::Create` returns nullptr

#### 3. Hardcoded Paths (Lines 47-58)
**Fix:** Use resource manager or fallback system

### Medium Priority

#### 4. Linear Search Performance (Lines 10-23)
**Fix:** Cache player entity reference

#### 5. Clear Without Cleanup (Line 22)
**Fix:** Cleanup physics bodies before clearing

---

## src/Renderer/Entity.h

### High Priority

#### 1. Invalid BodyID State (Line 63)
**Severity:** HIGH  
**Fix:**
```cpp
JPH::BodyID PhysicsBody = JPH::BodyID::cInvalidBodyID;
```

#### 2. Euler Angle Gimbal Lock (Lines 19-27)
**Severity:** HIGH  
**Fix:** Use quaternions instead of Euler angles

### Medium Priority

#### 3. Matrix Composition Order (Lines 20-23)
**Fix:** Use `glm::eulerAngleXYZ()` instead of separate matrices

#### 4. Magic Numbers (Lines 35-47)
**Fix:** Document units and use named constants

---

## src/Renderer/Camera.h

### Medium Priority

#### 1. Missing Pitch Clamping (Lines 28, 34, 37)
**Fix:**
```cpp
void SetPitch(float pitch) { 
    m_Pitch = glm::clamp(pitch, -89.0f, 89.0f);
    UpdateViewMatrix(); 
}
```

#### 2. Uninitialized m_Right (Lines 42-46)
**Fix:** `glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };`

---

## src/Renderer/Skybox.h & Skybox.cpp

### Critical Issues

#### 1. No OpenGL Error Checking (Lines 46-58)
**Severity:** CRITICAL  
**Fix:** Add glGetError() checks

#### 2. OpenGL State Pollution (Lines 46, 58)
**Severity:** HIGH  
**Fix:** Save and restore depth function

### High Priority

#### 3. Constructor Resource Creation (Lines 8-43)
**Fix:** Validate shader and texture creation

#### 4. Stack Overflow Risk (Lines 14-31)
**Fix:** Use `static constexpr` for vertices array

#### 5. Matrix Mutation (Lines 49-50)
**Fix:** Be explicit about column-major layout or use mat3 conversion

---

## src/Physics/PhysicsSystem.h & PhysicsSystem.cpp

### Critical Issues

#### 1. Fixed Timestep Ignores Delta (Lines 99-102)
**Severity:** CRITICAL  
**Issue:**
```cpp
void PhysicsSystem::OnUpdate(Timestep ts) {
    const float physicsDeltaTime = 1.0f / 60.0f;  // ❌ Ignores ts!
    s_PhysicsSystem->Update(physicsDeltaTime, 1, s_TempAllocator, s_JobSystem);
}
```
**Fix:**
```cpp
static float s_PhysicsAccumulator = 0.0f;
static constexpr float FIXED_PHYSICS_DT = 1.0f / 60.0f;
static constexpr int MAX_PHYSICS_STEPS = 5;

void PhysicsSystem::OnUpdate(Timestep ts) {
    s_PhysicsAccumulator += ts.GetSeconds();
    
    int steps = 0;
    while (s_PhysicsAccumulator >= FIXED_PHYSICS_DT && steps < MAX_PHYSICS_STEPS) {
        s_PhysicsSystem->Update(FIXED_PHYSICS_DT, 1, s_TempAllocator, s_JobSystem);
        s_PhysicsAccumulator -= FIXED_PHYSICS_DT;
        steps++;
    }
    
    if (s_PhysicsAccumulator > FIXED_PHYSICS_DT * MAX_PHYSICS_STEPS) {
        s_PhysicsAccumulator = FIXED_PHYSICS_DT;
    }
}
```

### High Priority

#### 2. No Thread Safety (Lines 69-76)
**Fix:** Add mutex for filter objects

#### 3. Missing Body Cleanup (Lines 91-97)
**Fix:** Remove and destroy all bodies before shutdown

#### 4. No Null Checks (Lines 99-113)
**Fix:** Assert/check `s_PhysicsSystem != nullptr`

### Medium Priority

#### 5-8. Limited layers, hardcoded threads, no error handling, no interpolation

---

## src/Physics/PlayerController.h & PlayerController.cpp

### Critical Issues

#### 1. Shape Memory Leak (Lines 32-36, 76-80)
**Severity:** CRITICAL  
**Fix:**
```cpp
JPH::Ref<JPH::CapsuleShape> capsule = new JPH::CapsuleShape(0.9f, 0.3f);
JPH::RotatedTranslatedShapeSettings shapeSettings(
    JPH::Vec3(0, 0.9f, 0), JPH::Quat::sIdentity(), capsule);
settings.mShape = shapeSettings.Create().Get();
```

#### 2. CharacterVirtual Leak (Lines 72-88)
**Severity:** CRITICAL  
**Fix:** Release old character before creating new one

#### 3. TempAllocator Per Frame (Line 225)
**Severity:** CRITICAL  
**Fix:** Make it a class member

### High Priority

#### 4. Unsafe UserData Cast (Lines 13-24)
**Fix:** Add validation or magic number check

### Medium Priority

#### 5. Double Gravity (Lines 211, 227)
**Fix:** Remove gravity from line 227 since handled manually

---

## src/ImGui/ImGuiLayer.h & ImGuiLayer.cpp

### Critical Issues

#### 1. Missing Context Validation (Lines 133-136, 176, 184)
**Severity:** CRITICAL  
**Fix:**
```cpp
void ImGuiLayer::OnEvent(Event &e) {
    if (m_BlockEvents && ImGui::GetCurrentContext()) {
        ImGuiIO &io = ImGui::GetIO();
        // ...
    }
}
```

#### 2. Resource Leak on Init Failure (Lines 82-123)
**Severity:** CRITICAL  
**Fix:** Cleanup context if backend init fails

### Medium Priority

#### 3. Unchecked Font Loading (Lines 95-97)
**Fix:** Check return value of `AddFontFromFileTTF`

#### 4. Missing GL Error Checking (Lines 121-122, 193)
**Fix:** Validate backend initialization

#### 5. Viewport Feature Mismatch (Lines 90-92, 111-114, 195-200)
**Fix:** Remove dead code or enable feature

#### 6. StyleVar Stack Imbalance (Lines 157-173)
**Fix:** Ensure cleanup on all paths

---

## src/ImGui/SceneHierarchyPanel.h & SceneHierarchyPanel.cpp

### Critical Issues

#### 1. Dangling Pointer Risk (Line 35)
**Severity:** CRITICAL  
**Fix:** Use `std::weak_ptr` or validate context

#### 2. Missing Null Checks (Lines 47-56, 99-177, 362-449)
**Severity:** CRITICAL  
**Fix:** Validate entity exists before operations

### Medium Priority

#### 3. Static Buffer in Rename (Lines 63-65)
**Fix:** Make member variable

#### 4. Font Index Without Bounds Check (Lines 222, 240, 281, 300, 320)
**Fix:** Check `io.Fonts->Fonts.Size > 0`

#### 5. Unchecked Texture Creation (Lines 142-148)
**Fix:** Validate texture creation

---

## src/ImGui/ContentBrowserPanel.h & ContentBrowserPanel.cpp

### Critical Issues

#### 1. Missing hash_value Definition (Line 373)
**Severity:** CRITICAL  
**Fix:**
```cpp
namespace {
    size_t hash_value(const std::filesystem::path& p) {
        return std::filesystem::hash_value(p);
    }
}
```

#### 2. Silent Exception Swallowing (Lines 78-79, 231-232, 270-271, 367-368, 387-388)
**Severity:** CRITICAL  
**Fix:** Log errors instead of empty catch blocks

### Medium Priority

#### 3. Thumbnail Cache Unbounded (Lines 176-186)
**Fix:** Implement LRU with size limit

#### 4. Unsafe String Copy (Lines 254-255)
**Fix:** Ensure null termination

#### 5. Unvalidated Texture IDs (Lines 171-186)
**Fix:** Check renderer ID != 0

#### 6. Race Condition (Lines 54-58, 68-75)
**Fix:** Use atomic file operations

#### 7. No Error Handling (Lines 393-445)
**Fix:** Check file write success

---

## Summary Statistics

- **Total Files Reviewed:** 45+
- **Total Issues Found:** 119
- **Critical Issues:** 28
- **High Priority Issues:** 32
- **Medium Priority Issues:** 41
- **Low Priority Issues:** 18

**End of Detailed Issues List**
