-- Vertical Circle + Horizontal Spin Script

local time = 0.0
local centerPos = vec3(0,0,0)
local radius = 2.0
local moveSpeed = 2.0
local spinSpeed = 90.0

function onCreate()
    -- Lock physics so we can control movement perfectly (Kinematic)
    self:setAnchored(true)
    centerPos = self:getPosition()
end

function onUpdate(ts)
    time = time + ts
    
    -- Vertical Circle (Y and X axes)
    local x = math.cos(time * moveSpeed) * radius
    local y = math.sin(time * moveSpeed) * radius
    
    self:setPosition(centerPos + vec3(x, y, 0))
    
    -- Spin Horizontally (Y axis)
    local currentRot = self:getRotation()
    currentRot.y = currentRot.y + (spinSpeed * ts)
    self:setRotation(currentRot)
end
