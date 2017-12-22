

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

