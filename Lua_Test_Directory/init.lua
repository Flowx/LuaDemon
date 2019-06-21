

_TEST = _TEST or 0

_TEST = _TEST + 1

print("Test: ".._TEST)
print("Dir: " .. _LUAENV.ENVDIR)
print("System: " .. _LUAENV.ENVSYS)


--[[
for k,v in pairs(_G) do
	print(k,v)
end
]]



include("jsonparser.lua")

include("intellicon/init.lua")




print("Done!")















































-- net.openUDP(696, function()
	-- print("call 2!")
-- end)







