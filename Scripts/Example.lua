-- Example Lua Script
-- Save this as Scripts/MyScript.lua

function onCreate()
    printHUD("Lua Script Loaded!", vec4(0, 1, 0, 1))
end

function onUpdate(ts)
    -- self is the entity this script is attached to
    
    local hit = raycast(10.0)
    
    if hit then
        setText("Interaction", "[E] " .. hit:getName(), vec2(0.5, 0.1))
        
        if isKeyPressed(KEY_E) then
            log("Key E Pressed against " .. hit:getName())
            -- Move the object we hit up!
            local pos = hit:getPosition()
            pos.y = pos.y + 0.1
            hit:setPosition(pos)
        end
    else
        clearText("Interaction")
    end
end
