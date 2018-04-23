
_TEST = _TEST or 0

include("duh/includeme.lua")
print("Test: ".._TEST)
print("Test! Dir: " .. _LUAENV.ENVDIR)
print("Test! System: " .. _LUAENV.ENVSYS)

local PORT_A = "ttyS0"

serial.Open(PORT_A)

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
