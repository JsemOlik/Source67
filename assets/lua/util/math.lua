-- Math Utility Functions for Lua Scripts

local MathUtil = {}

-- Clamp value between min and max
function MathUtil.Clamp(value, min, max)
    if value < min then return min end
    if value > max then return max end
    return value
end

-- Linear interpolation
function MathUtil.Lerp(a, b, t)
    return a + (b - a) * t
end

-- Smooth step interpolation
function MathUtil.SmoothStep(edge0, edge1, x)
    local t = MathUtil.Clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)
end

-- Distance between two 2D points
function MathUtil.Distance2D(x1, y1, x2, y2)
    local dx = x2 - x1
    local dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)
end

-- Distance between two 3D points
function MathUtil.Distance3D(x1, y1, z1, x2, y2, z2)
    local dx = x2 - x1
    local dy = y2 - y1
    local dz = z2 - z1
    return math.sqrt(dx * dx + dy * dy + dz * dz)
end

-- Normalize 2D vector
function MathUtil.Normalize2D(x, y)
    local length = math.sqrt(x * x + y * y)
    if length > 0 then
        return x / length, y / length
    end
    return 0, 0
end

-- Normalize 3D vector
function MathUtil.Normalize3D(x, y, z)
    local length = math.sqrt(x * x + y * y + z * z)
    if length > 0 then
        return x / length, y / length, z / length
    end
    return 0, 0, 0
end

-- Dot product (3D)
function MathUtil.Dot3D(x1, y1, z1, x2, y2, z2)
    return x1 * x2 + y1 * y2 + z1 * z2
end

-- Random float between min and max
function MathUtil.RandomRange(min, max)
    return min + math.random() * (max - min)
end

-- Random integer between min and max (inclusive)
function MathUtil.RandomInt(min, max)
    return math.random(min, max)
end

-- Convert degrees to radians
function MathUtil.DegToRad(degrees)
    return degrees * (math.pi / 180.0)
end

-- Convert radians to degrees
function MathUtil.RadToDeg(radians)
    return radians * (180.0 / math.pi)
end

-- Check if value is approximately equal
function MathUtil.ApproximatelyEqual(a, b, epsilon)
    epsilon = epsilon or 0.0001
    return math.abs(a - b) < epsilon
end

return MathUtil
