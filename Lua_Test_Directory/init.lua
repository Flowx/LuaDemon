
do -- Startup Crap
_G._TESTCYCLE = _G._TESTCYCLE or 0
_G._TESTCYCLE = _G._TESTCYCLE + 1

print("Run # " .. _TESTCYCLE)
print("Root: " .. _LUAENV.ENVDIR)
print("Arch: " .. _LUAENV.ENVSYS)
print("\n")

-- This overloads the default type() function
-- so it accept alternative names using __type
if not _G.__oldtype then -- just check if its already overloaded
	_G.__oldtype = _G.type
	_G.type = nil
	_G.type = function( v )
		local t = getmetatable(v)
		if t and t.__type then return t.__type end
		return _G.__oldtype(v)
	end
end

end


think.add("mytest", 500, function()
	print("tick!\n")
end)

think.remove("mytest")



local t = think.list()

for k,v in pairs(t) do
	print( v.identifier .. " : " .. v.interval .. "ms\n")
end


-- _G.sock = tcp.open(1337, false, function(data, ip)
	-- print("DATA! From: " .. ip)
	-- print("\nData: " .. data)
	
	-- print(sock:list())
	
	-- for k,v in pairs(sock:list()) do 
		-- print(v)
		
		-- v:send("aaa\\aaa\0aiaiushbd")
		
		-- print("\nValid: " .. tostring(v:isValid()))
		-- print("\nStill valid: " .. tostring(v:isValid("lellelele")))
	-- end
	
	
	
-- end)

-- if sock and sock:isValid() then
	-- print("TCP Socket on Port: " .. sock:getPort() .. " is ready\n")
-- else
	-- print("Failed to open socket!\n")
-- end




--print(sock)
--include("intellicon_host/init.lua")



