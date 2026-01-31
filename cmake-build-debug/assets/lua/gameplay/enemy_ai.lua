-- Enemy AI Lua Script
-- Demonstrates enemy behavior and AI logic in Lua

local EnemyAI = {}

-- AI States
local AIState = {
    IDLE = "idle",
    PATROL = "patrol",
    CHASE = "chase",
    ATTACK = "attack",
    RETREAT = "retreat"
}

function EnemyAI:OnCreate()
    self.speed = 3.0
    self.health = 50
    self.maxHealth = 50
    self.detectionRadius = 10.0
    self.attackRange = 2.0
    self.attackDamage = 10.0
    self.attackCooldown = 1.0
    self.currentCooldown = 0.0
    
    self.state = AIState.IDLE
    self.target = nil
    
    print("[Lua] EnemyAI created - State: " .. self.state)
end

function EnemyAI:OnUpdate(dt)
    self.currentCooldown = math.max(0, self.currentCooldown - dt)
    
    -- State machine
    if self.state == AIState.IDLE then
        self:UpdateIdle(dt)
    elseif self.state == AIState.PATROL then
        self:UpdatePatrol(dt)
    elseif self.state == AIState.CHASE then
        self:UpdateChase(dt)
    elseif self.state == AIState.ATTACK then
        self:UpdateAttack(dt)
    elseif self.state == AIState.RETREAT then
        self:UpdateRetreat(dt)
    end
end

function EnemyAI:UpdateIdle(dt)
    -- Look for player
    -- local player = FindEntityWithTag("Player")
    -- if player and DistanceTo(player) < self.detectionRadius then
    --     self:SetTarget(player)
    --     self:ChangeState(AIState.CHASE)
    -- end
end

function EnemyAI:UpdatePatrol(dt)
    -- Simple patrol logic
    -- Move along waypoints
end

function EnemyAI:UpdateChase(dt)
    if not self.target then
        self:ChangeState(AIState.IDLE)
        return
    end
    
    -- Move towards target
    -- local distance = DistanceTo(self.target)
    -- if distance <= self.attackRange then
    --     self:ChangeState(AIState.ATTACK)
    -- elseif distance > self.detectionRadius * 1.5 then
    --     self:ChangeState(AIState.IDLE)
    -- end
end

function EnemyAI:UpdateAttack(dt)
    if not self.target then
        self:ChangeState(AIState.IDLE)
        return
    end
    
    if self.currentCooldown <= 0 then
        self:PerformAttack()
        self.currentCooldown = self.attackCooldown
    end
    
    -- Check if target out of range
    -- local distance = DistanceTo(self.target)
    -- if distance > self.attackRange then
    --     self:ChangeState(AIState.CHASE)
    -- end
end

function EnemyAI:UpdateRetreat(dt)
    -- Retreat when low health
    if self.health > self.maxHealth * 0.3 then
        self:ChangeState(AIState.CHASE)
    end
end

function EnemyAI:PerformAttack()
    print("[Lua] Enemy attacking! Damage: " .. self.attackDamage)
    -- Apply damage to target
    -- if self.target then
    --     self.target:TakeDamage(self.attackDamage)
    -- end
end

function EnemyAI:TakeDamage(amount)
    self.health = self.health - amount
    print("[Lua] Enemy took damage: " .. amount .. " (Health: " .. self.health .. ")")
    
    if self.health <= 0 then
        self:Die()
    elseif self.health < self.maxHealth * 0.3 then
        self:ChangeState(AIState.RETREAT)
    end
end

function EnemyAI:Die()
    print("[Lua] Enemy died!")
    -- Drop loot, award points, play death animation
    -- self:Destroy()
end

function EnemyAI:SetTarget(target)
    self.target = target
    print("[Lua] Enemy acquired target")
end

function EnemyAI:ChangeState(newState)
    print("[Lua] Enemy state change: " .. self.state .. " -> " .. newState)
    self.state = newState
end

function EnemyAI:OnDestroy()
    print("[Lua] EnemyAI destroyed")
end

return EnemyAI
