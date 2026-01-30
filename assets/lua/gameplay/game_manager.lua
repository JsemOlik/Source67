-- Game Manager Lua Script
-- Manages overall game state, scoring, level progression

local GameManager = {}

function GameManager:OnCreate()
    self.score = 0
    self.level = 1
    self.enemiesDefeated = 0
    self.gameTime = 0.0
    self.isPaused = false
    
    print("[Lua] GameManager initialized")
    print("  Level: " .. self.level)
    print("  Score: " .. self.score)
end

function GameManager:OnUpdate(dt)
    if self.isPaused then
        return
    end
    
    self.gameTime = self.gameTime + dt
end

function GameManager:AddScore(points)
    self.score = self.score + points
    print("[Lua] Score added: " .. points .. " (Total: " .. self.score .. ")")
    
    -- Check for level progression
    if self.score >= self.level * 1000 then
        self:NextLevel()
    end
end

function GameManager:OnEnemyDefeated()
    self.enemiesDefeated = self.enemiesDefeated + 1
    self:AddScore(100)
    
    print("[Lua] Enemy defeated! Total: " .. self.enemiesDefeated)
end

function GameManager:NextLevel()
    self.level = self.level + 1
    print("[Lua] Level up! Now at level: " .. self.level)
    
    -- Load next level, increase difficulty, etc.
end

function GameManager:PauseGame()
    self.isPaused = true
    print("[Lua] Game paused")
end

function GameManager:ResumeGame()
    self.isPaused = false
    print("[Lua] Game resumed")
end

function GameManager:ResetGame()
    self.score = 0
    self.level = 1
    self.enemiesDefeated = 0
    self.gameTime = 0.0
    self.isPaused = false
    
    print("[Lua] Game reset")
end

function GameManager:GetGameStats()
    return {
        score = self.score,
        level = self.level,
        enemiesDefeated = self.enemiesDefeated,
        gameTime = self.gameTime
    }
end

return GameManager
