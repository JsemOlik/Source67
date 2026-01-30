-- Example Lua Script
-- Save this as Scripts/MyScript.lua

function onCreate()
    printHUD("Lua Script Loaded!", vec4(0, 1, 0, 1))
end

local useSmooth = true -- Change to false for Instant Teleport

function onUpdate(ts)
    -- self is the entity this script is attached to
    
    local hit = raycast(10.0)
    
    if hit then
        if useSmooth then
            setText("Interaction", "[Hold E] Smooth Move " .. hit:getName(), vec2(0.5, 0.1))
            if isKeyPressed(KEY_E) then
                -- Apply velocity to fight gravity and move up
                hit:setLinearVelocity(vec3(0, 4.0, 0))
            end
        else
            setText("Interaction", "[Click E] Instant Teleport " .. hit:getName(), vec2(0.5, 0.1))
            if isKeyJustPressed(KEY_E) then
                log("Teleporting " .. hit:getName())
                local pos = hit:getPosition()
                pos.y = pos.y + 1.0
                hit:setPosition(pos)
                hit:setLinearVelocity(vec3(0, 0, 0)) -- Stop potential momentum
            end
        end
    else
        clearText("Interaction")
    end
end
