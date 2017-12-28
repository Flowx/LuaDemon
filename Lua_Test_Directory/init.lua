
_TEST = _TEST or 0

include("duh/includeme.lua")
print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)

local s = serial.Open("COM8", 9600)

serial.Receive("COM8", function(Data)
	print("got something")
end)


do -- Test delay
	local count = 0
	while true do
		serial.Send("COM8", 65)
		for i=1,5000000 do
			local a = {}
		end
		serial.Send("COM8", 66)
		count = count + 1
		print("\nCount: " .. count)
		if count == 18 then break end
	end
end