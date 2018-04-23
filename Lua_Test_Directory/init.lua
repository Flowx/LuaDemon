
_TEST = _TEST or 0

include("duh/includeme.lua")
print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)

local PORT_A = "ttyS0"

serial.Open(PORT_A)

<<<<<<< HEAD
-- local activeports = serial.ListOpen()
-- for k,v in pairs(activeports) do
	-- print("Active: "..v)
-- end


-- local activeports = serial.ListOpen()
-- for k,v in pairs(activeports) do
	-- print("Active: "..v)
-- end
serial.Send(PORT_A, "I LIKE CHEESE!")

print(serial.ReadAll(PORT_A))
=======
serial.Open("COM4", 115200)

local a = "E"

serial.Send("COM4", a)



local out = serial.ReadAll("COM4")
print(out)

local dayta = serial.Read("COM4")

local count = 0

serial.Receive("COM4", function()
	count = count + 1
	local out = serial.ReadAll("COM4")
	print(out)

	serial.Send("COM4", "nignog")
	if(count > 1000) then count = -10 end
end)


>>>>>>> 3c6f26d9329116d9312661608b4db857b6339019
