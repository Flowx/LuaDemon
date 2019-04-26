
_TEST = _TEST or 0
_TEST = _TEST + 1

include("duh/includeme.lua")
print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)

local PORT_A = "ttyS0"

local s = 0
s = net.openTCP(666, 5, function()
	print("call tcp!")
end)
print(s)















































-- net.openUDP(696, function()
	-- print("call 2!")
-- end)







