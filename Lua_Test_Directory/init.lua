
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


print("\n\n\nInitializing IntelliCon Host\n")

udp.open(1337, false, function(data, ip)
	print(data[1])
	if data[0] == '$' then -- this is a broadcast
		print("Received Broadcast from :" .. ip .. "\n")
		
	else 
	
	end

end)

