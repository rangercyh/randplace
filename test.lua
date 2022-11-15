local randplace = require "randplace"


--[[
print(collectgarbage("count"))
for i = 1, 1000 do
    local map = randplace.new(0, 0, 100, 100)
    map:dighole(0, 0, 10, 10)
    map:dighole(0, 0, 20, 20)
    map:randplace(20, 20)
end
print(collectgarbage("count"))
collectgarbage("collect")
collectgarbage("collect")
print(collectgarbage("count"))
]]
local map = randplace.new(-10, -10, 20, 20)
print("dig hole = ", map:dighole(0, 0, 10, 10))
print("dig hole = ", map:dighole(-10, -10, 5, 5))
for i = 1, 10 do
    print(map:randplace(5, 5))
end

local map = randplace.new(0, 0, 9, 9)
print("dig hole = ", map:dighole(3, 0, 3, 3))
print("dig hole = ", map:dighole(0, 3, 3, 3))
print("dig hole = ", map:dighole(3, 6, 3, 3))
print("dig hole = ", map:dighole(6, 3, 3, 3))
for i = 1, 6 do
    local x, y = map:randplace(3, 3)
    print(x, y)
    if x then
        print("dig hole = ", map:dighole(x, y, 3, 3))
    end
end

print("del hole = ", map:delhole(9))
print(map:randplace(3, 3))
print(map:randplace(3, 3))

-- for i = 1, 10 do
--     local x, y = map:randplace(5, 5)
--     map:dighole(x, y, 5, 5)
--     print(i, x, y)
-- end

-- local holes = {
--     { x = 450, y = 74, w = 222, h = 174 },
--     { x = 116, y = 103, w = 182, h = 161 },
--     { x = 383, y = 348, w = 244, h = 134 },
--     { x = 43, y = 1, w = 50, h = 50 },
-- }
-- local n = 10
-- local w = 50
-- local h = 50
-- local map = randplace.new(0, 0, 800, 550)
-- for i = 1, #holes do
--     map:dighole(holes[i].x, holes[i].y, holes[i].w, holes[i].h)
-- end
-- local r = {}
-- for i = 1, n do
--     local x, y = map:randplace(w, h)
--     if x then
--         map:dighole(x, y, w, h)
--         r[#r + 1] = { x, y }
--         print('===', x, y)
--     end
-- end


