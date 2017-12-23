
_TEST = _TEST or 0

include("includeme.lua")


print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)

local count = 0
while true do
	for i=1,1000000 do
		local a = {}
	end
	count = count + 1
	print("\nCount: " .. count)
	if count == 5 then break end
end

