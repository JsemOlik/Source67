-- Player Controller Lua Script
-- This script demonstrates Lua gameplay logic integration with Source67

local PlayerController = {}

function PlayerController:OnCreate()
    self.speed = 5.0
    self.health = 100
    self.maxHealth = 100
    self.isAlive = true
    
    print("[Lua] PlayerController created")
    print("  Speed: " .. self.speed)
    print("  Health: " .. self.health)
end

function PlayerController:OnUpdate(dt)
    if not self.isAlive then
        return
    end
    
    -- Example: Health regeneration
    if self.health < self.maxHealth then
        self.health = math.min(self.health + 5.0 * dt, self.maxHealth)
    end
    
    -- Input handling would go here (via engine API)
    -- Example:
    -- if IsKeyPressed(KEY_W) then
    --     local transform = self:GetComponent("Transform")
    --     transform.position.z = transform.position.z + self.speed * dt
    -- end
end

function PlayerController:TakeDamage(amount)
    if not self.isAlive then
        return
    end
    
    self.health = self.health - amount
    print("[Lua] Player took damage: " .. amount .. " (Health: " .. self.health .. ")")
    
    if self.health <= 0 then
        self:Die()
    end
end

function PlayerController:Heal(amount)
    if not self.isAlive then
        return
    end
    
    local oldHealth = self.health
    self.health = math.min(self.health + amount, self.maxHealth)
    local healed = self.health - oldHealth
    
    print("[Lua] Player healed: " .. healed .. " (Health: " .. self.health .. ")")
end

function PlayerController:Die()
    self.isAlive = false
    self.health = 0
    print("[Lua] Player died!")
    
    -- Trigger death animation, respawn logic, etc.
end

function PlayerController:OnDestroy()
    print("[Lua] PlayerController destroyed")
end

return PlayerController
