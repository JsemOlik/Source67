-- Helper Utility Functions

local Helpers = {}

-- Deep copy a table
function Helpers.DeepCopy(original)
    local copy
    if type(original) == 'table' then
        copy = {}
        for k, v in pairs(original) do
            copy[Helpers.DeepCopy(k)] = Helpers.DeepCopy(v)
        end
        setmetatable(copy, Helpers.DeepCopy(getmetatable(original)))
    else
        copy = original
    end
    return copy
end

-- Check if table contains value
function Helpers.Contains(table, value)
    for _, v in pairs(table) do
        if v == value then
            return true
        end
    end
    return false
end

-- Get table size (works for non-sequential tables)
function Helpers.TableSize(table)
    local count = 0
    for _ in pairs(table) do
        count = count + 1
    end
    return count
end

-- Print table contents (for debugging)
function Helpers.PrintTable(table, indent)
    indent = indent or ""
    for k, v in pairs(table) do
        if type(v) == "table" then
            print(indent .. tostring(k) .. ":")
            Helpers.PrintTable(v, indent .. "  ")
        else
            print(indent .. tostring(k) .. " = " .. tostring(v))
        end
    end
end

-- Simple timer class
Helpers.Timer = {}
Helpers.Timer.__index = Helpers.Timer

function Helpers.Timer.new(duration, callback)
    local timer = setmetatable({}, Helpers.Timer)
    timer.duration = duration
    timer.elapsed = 0
    timer.callback = callback
    timer.active = true
    timer.repeat_count = 0
    return timer
end

function Helpers.Timer:update(dt)
    if not self.active then
        return
    end
    
    self.elapsed = self.elapsed + dt
    if self.elapsed >= self.duration then
        if self.callback then
            self.callback()
        end
        self.elapsed = 0
        self.repeat_count = self.repeat_count + 1
        return true -- Timer completed
    end
    return false
end

function Helpers.Timer:reset()
    self.elapsed = 0
    self.repeat_count = 0
end

function Helpers.Timer:stop()
    self.active = false
end

function Helpers.Timer:start()
    self.active = true
end

-- String utilities
function Helpers.Split(str, delimiter)
    local result = {}
    local pattern = string.format("([^%s]+)", delimiter)
    for match in string.gmatch(str, pattern) do
        table.insert(result, match)
    end
    return result
end

function Helpers.Trim(str)
    return str:match("^%s*(.-)%s*$")
end

-- Color utilities
function Helpers.ColorFromHex(hex)
    hex = hex:gsub("#", "")
    local r = tonumber("0x" .. hex:sub(1, 2)) / 255
    local g = tonumber("0x" .. hex:sub(3, 4)) / 255
    local b = tonumber("0x" .. hex:sub(5, 6)) / 255
    local a = 1.0
    if #hex == 8 then
        a = tonumber("0x" .. hex:sub(7, 8)) / 255
    end
    return {r, g, b, a}
end

return Helpers
