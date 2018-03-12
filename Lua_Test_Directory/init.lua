
_TEST = _TEST or 0

include("duh/includeme.lua")
print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)



local ports = serial.Discover()
for k,v in pairs(ports) do
	print("Port: "..v)
end

local PORT_A = _LUAENV.ENVSYS == "LINUX" and "ttyS0" or "COM1"
serial.Open(PORT_A, 9600)
serial.Receive(PORT_A, function(Data)
	--print("got something\n")
	local a = serial.ReadAll(PORT_A)
	--a = nil
	print(type(a))
	print("Data: " .. a)
end)

local activeports = serial.ListOpen()
print(activeports)
for k,v in pairs(activeports) do
	print("Active: "..v)
end


-- print(serial)
-- serial.Open(PORT_B, 9600)
-- -- serial.Receive(PORT_B, function(Data)
	-- -- --print("got something else a\n")
	-- -- print("Data: " .. serial.ReadAll(PORT_B))
-- -- end)
-- serial.Receive(PORT_B)

do -- Test delay
	local count = 0
	while true do
		serial.Send(PORT_A, 65)
		-- serial.Send(PORT_B, 66)
		for i=1,1000000 do local a = {} end
		
		-- print("Available A: " .. serial.Available(PORT_A))
		-- print("Data: " .. serial.ReadAll(PORT_A))
		-- print("Available B: " .. serial.Available(PORT_B))
		-- print("Data: " .. serial.ReadAll(PORT_B))
		
		serial.Send(PORT_A, 66)
		-- serial.Send(PORT_B, 65)
		for i=1,1000000 do local a = {} end
		
		print("Available A: " .. serial.Available(PORT_A))
		-- print("Data: " .. serial.ReadAll(PORT_A))
		-- print("Available B: " .. serial.Available(PORT_B))
		-- print("Data: " .. serial.ReadAll(PORT_))
		
		print("\nCount: " .. count)
		
		count = count + 1
		if count == 18 then break end
	end
end
