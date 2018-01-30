
_TEST = _TEST or 0

include("duh/includeme.lua")
print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)

local PORT = "COM4"
serial.Open(PORT, 9600)
serial.Receive(PORT, function(Data)
	print("got something\n")
end)

do -- Test delay
	local count = 0
	while true do
		serial.Send(PORT, 65)
		for i=1,3000000 do local a = {} end
		
		serial.Send(PORT, 66)
		
		for i=1,3000000 do local a = {} end
		
		count = count + 1
		print("\nCount: " .. count)
		print("Available: " .. serial.Available(PORT))
		if count == 18 then break end
	end
end
