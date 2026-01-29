# Source67 - Critical Issues Checklist

**Total Critical Issues: 28**  
**Estimated Fix Time: 6-8 hours**  
**Priority: Fix before next milestone**

---

## ✅ Immediate Fixes (45 minutes)

### [ ] 1. Fix main.cpp Memory Leak
- **File:** `main.cpp:31-33`
- **Time:** 5 minutes
- **Fix:** Replace raw pointer with `std::make_unique`
- **Test:** Run app and verify clean shutdown

### [ ] 2. Fix Physics Timestep
- **File:** `PhysicsSystem.cpp:99-102`
- **Time:** 30 minutes
- **Fix:** Implement fixed timestep accumulator
- **Test:** Verify physics runs at 60 FPS regardless of framerate

### [ ] 3. Fix TempAllocator Allocation
- **File:** `PlayerController.cpp:225`
- **Time:** 5 minutes
- **Fix:** Move TempAllocator to member variable
- **Test:** Run profiler, verify allocation dropped

### [ ] 4. Add OpenGL Validation
- **File:** `Renderer.cpp:Init()`
- **Time:** 5 minutes
- **Fix:** Check GLAD initialization success
- **Test:** Force failure, verify graceful error

**Checkpoint:** Commit after these 4 fixes

---

## 🔴 OpenGL Resource Management (2-3 hours)

### [ ] 5. Shader - Implement Rule of Five
- **File:** `Shader.h` and `Shader.cpp`
- **Time:** 30 minutes
- **Changes:**
  ```cpp
  // Add to Shader.h
  OpenGLShader(const OpenGLShader&) = delete;
  OpenGLShader& operator=(const OpenGLShader&) = delete;
  OpenGLShader(OpenGLShader&& other) noexcept;
  OpenGLShader& operator=(OpenGLShader&& other) noexcept;
  ```
- **Test:** Create temp shader, verify no crashes

### [ ] 6. Texture - Implement Rule of Five
- **File:** `Texture.h` and `Texture.cpp`
- **Time:** 30 minutes
- **Changes:** Same pattern as Shader
- **Test:** Copy/move textures, verify no leaks

### [ ] 7. Buffer - Implement Rule of Five
- **File:** `Buffer.h` and `Buffer.cpp`
- **Time:** 20 minutes
- **Changes:** Same pattern as Shader
- **Test:** Create/destroy buffers, check GL errors

### [ ] 8. VertexArray - Implement Rule of Five
- **File:** `VertexArray.h` and `VertexArray.cpp`
- **Time:** 20 minutes
- **Changes:** Same pattern as Shader
- **Test:** Build mesh, verify no double-free

### [ ] 9. Framebuffer - Implement Rule of Five
- **File:** `Framebuffer.h` and `Framebuffer.cpp`
- **Time:** 30 minutes
- **Changes:** Same pattern as Shader
- **Test:** Resize window, verify FBO recreation

### [ ] 10. Skybox - Implement Rule of Five
- **File:** `Skybox.h` and `Skybox.cpp`
- **Time:** 20 minutes
- **Changes:** Same pattern as Shader
- **Test:** Load/unload skybox, verify cleanup

**Checkpoint:** Commit "Implement Rule of Five for all OpenGL resources"

---

## 🔴 Resource Initialization (1 hour)

### [ ] 11. Framebuffer - Initialize Members
- **File:** `Framebuffer.cpp:constructor`
- **Time:** 10 minutes
- **Fix:** Initialize m_RendererID to 0
- **Test:** Create FBO before window ready, verify no crash

### [ ] 12. Shader - Fix Compilation Leak
- **File:** `Shader.cpp:CompileShader()`
- **Time:** 30 minutes
- **Fix:** Delete shader on compilation error
- **Test:** Force shader error, verify no leak

### [ ] 13. Texture - Fix Loading Leak
- **File:** `Texture.cpp:constructor`
- **Time:** 20 minutes
- **Fix:** Delete GL texture on image load failure
- **Test:** Load invalid image, verify cleanup

**Checkpoint:** Commit "Fix resource initialization and error cleanup"

---

## 🔴 Thread Safety (1-2 hours)

### [ ] 14. Logger - Thread-Safe Log History
- **File:** `Logger.cpp:multiple locations`
- **Time:** 1 hour
- **Fix:** Add mutex around s_LogHistory access
- **Test:** Multi-threaded logging, verify no crashes

### [ ] 15. UndoSystem - Thread-Safe Stack Access
- **File:** `UndoSystem.cpp`
- **Time:** 30 minutes
- **Fix:** Add mutex for undo/redo operations
- **Test:** Rapid undo/redo from multiple inputs

### [ ] 16. PhysicsSystem - Thread-Safe Body Access
- **File:** `PhysicsSystem.cpp`
- **Time:** 30 minutes
- **Fix:** Use BodyLockInterface for all access
- **Test:** Create/destroy bodies while simulating

**Checkpoint:** Commit "Add thread safety to core systems"

---

## 🔴 Build Configuration (1 hour)

### [ ] 17. CMake - Pin Jolt Version
- **File:** `CMakeLists.txt:62`
- **Time:** 5 minutes
- **Fix:** Change `GIT_TAG master` to `GIT_TAG v5.0.0`
- **Test:** Clean build, verify specific version

### [ ] 18. CMake - Pin GLAD Version
- **File:** `CMakeLists.txt:92`
- **Time:** 5 minutes
- **Fix:** Pin to specific commit SHA
- **Test:** Clean build, verify reproducible

### [ ] 19. CMake - Add Compiler Warnings
- **File:** `CMakeLists.txt` after line 16
- **Time:** 10 minutes
- **Fix:** Add -Wall -Wextra -Wpedantic
- **Test:** Build, fix new warnings

### [ ] 20. CMake - Add Linux OpenGL Linking
- **File:** `CMakeLists.txt:160-162`
- **Time:** 10 minutes
- **Fix:** Add `find_package(OpenGL)` for Linux
- **Test:** Build on Linux

**Checkpoint:** Commit "Improve CMake configuration"

---

## 🔴 Error Handling (1 hour)

### [ ] 21. Window - Check GLFW Init
- **File:** `Window.cpp:constructor`
- **Time:** 10 minutes
- **Fix:** Verify glfwInit() returns true
- **Test:** Force GLFW failure

### [ ] 22. Window - Add glfwTerminate
- **File:** `Window.cpp:destructor`
- **Time:** 10 minutes
- **Fix:** Call glfwTerminate() in destructor
- **Test:** Create/destroy window multiple times

### [ ] 23. Mesh - Fix Aliasing Violation
- **File:** `Mesh.cpp:LoadOBJ()`
- **Time:** 20 minutes
- **Fix:** Use proper glm conversion functions
- **Test:** Load complex OBJ, verify correctness

### [ ] 24. Scene - Add Entity Existence Check
- **File:** `Scene.cpp:GetPlayer()`
- **Time:** 15 minutes
- **Fix:** Cache player entity reference
- **Test:** Delete player, verify no crash

**Checkpoint:** Commit "Add error handling and validation"

---

## 🔴 Physics Issues (30 minutes)

### [ ] 25. PhysicsSystem - Fix Gravity Sign
- **File:** `PhysicsSystem.cpp:99`
- **Time:** 5 minutes
- **Fix:** Use JPH::Vec3(0, -9.81f, 0)
- **Test:** Drop object, verify falls down

### [ ] 26. PhysicsSystem - Fix Collision Layer Init
- **File:** `PhysicsSystem.cpp:Init()`
- **Time:** 10 minutes
- **Fix:** Properly initialize layer pairs
- **Test:** Verify collisions work correctly

### [ ] 27. PlayerController - Fix Gravity Application
- **File:** `PlayerController.cpp:225`
- **Time:** 10 minutes
- **Fix:** Don't apply gravity twice
- **Test:** Jump mechanics feel correct

**Checkpoint:** Commit "Fix physics system issues"

---

## 🔴 ImGui Issues (30 minutes)

### [ ] 28. ContentBrowser - Validate Thumbnail Cache
- **File:** `ContentBrowserPanel.cpp`
- **Time:** 20 minutes
- **Fix:** Add cache size limit (e.g., 100 MB)
- **Test:** Browse 1000+ images, verify bounded memory

### [ ] 29. SceneHierarchy - Validate UserData Cast
- **File:** `SceneHierarchyPanel.cpp:OnDrop()`
- **Time:** 10 minutes
- **Fix:** Validate pointer before cast
- **Test:** Drop invalid data, verify no crash

**Checkpoint:** Commit "Fix ImGui panel issues"

---

## 📝 Testing Checklist

After fixing all critical issues, perform these tests:

### Functional Tests
- [ ] Start application (clean startup)
- [ ] Create new scene
- [ ] Add entities (cube, sphere, etc.)
- [ ] Move, rotate, scale entities
- [ ] Enter play mode
- [ ] Physics simulation runs correctly
- [ ] Player controller responds
- [ ] Exit play mode
- [ ] Save scene
- [ ] Load scene
- [ ] Exit application (clean shutdown)

### Stress Tests
- [ ] Create 1000 entities
- [ ] Enter/exit play mode 100 times
- [ ] Load 100 different scenes
- [ ] Run for 30 minutes continuous
- [ ] Monitor memory (should be stable)
- [ ] Check for OpenGL errors (should be none)

### Error Handling Tests
- [ ] Load invalid scene file
- [ ] Load corrupt image texture
- [ ] Compile invalid shader
- [ ] Delete required assets
- [ ] Force window creation failure
- [ ] Disconnect monitor during fullscreen

### Performance Tests
- [ ] Profile frame time (should be <16ms)
- [ ] Check allocation rate (should be <100 MB/sec)
- [ ] Verify 60 FPS in editor
- [ ] Verify 60 FPS in play mode
- [ ] Check physics timestep accuracy

---

## 🎯 Success Criteria

All critical issues fixed when:

- ✅ No crashes during normal operation
- ✅ No memory leaks (verified with sanitizers)
- ✅ No OpenGL errors (verified with debug context)
- ✅ Physics runs at stable 60 FPS
- ✅ Thread-safe logging works
- ✅ All resources properly cleaned up
- ✅ Build is reproducible (pinned dependencies)
- ✅ Compiler warnings enabled and fixed

---

## 📊 Progress Tracking

- **Issues Fixed:** [ ] / 28
- **Time Spent:** _____ / 6-8 hours
- **Tests Passed:** [ ] / 22
- **Ready for Production:** [ ] Yes / [x] Not Yet

---

**Last Updated:** [Date]  
**Assigned To:** [Developer]  
**Review Status:** [ ] In Progress / [ ] Complete
