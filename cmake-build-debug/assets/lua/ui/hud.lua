-- HUD (Heads-Up Display) Lua Script
-- Manages UI rendering and updates

local HUD = {}

function HUD:OnCreate()
    self.visible = true
    self.showFPS = true
    self.showHealth = true
    self.showScore = true
    
    print("[Lua] HUD created")
end

function HUD:OnUpdate(dt)
    if not self.visible then
        return
    end
    
    -- In production, would call engine's HUDRenderer
    -- Example:
    -- if self.showHealth then
    --     local player = GetPlayer()
    --     if player then
    --         DrawText("Health: " .. player.health, 10, 10, {1, 0, 0, 1})
    --     end
    -- end
    --
    -- if self.showScore then
    --     local gameManager = GetGameManager()
    --     if gameManager then
    --         DrawText("Score: " .. gameManager.score, 10, 30, {1, 1, 1, 1})
    --     end
    -- end
    --
    -- if self.showFPS then
    --     local fps = GetFPS()
    --     DrawText("FPS: " .. fps, 10, 50, {0, 1, 0, 1})
    -- end
end

function HUD:SetVisible(visible)
    self.visible = visible
    print("[Lua] HUD visibility: " .. tostring(visible))
end

function HUD:ToggleElement(element)
    if element == "fps" then
        self.showFPS = not self.showFPS
    elseif element == "health" then
        self.showHealth = not self.showHealth
    elseif element == "score" then
        self.showScore = not self.showScore
    end
end

function HUD:ShowMessage(message, duration)
    print("[Lua] HUD Message: " .. message .. " (duration: " .. duration .. "s)")
    -- Display temporary message on screen
end

return HUD
