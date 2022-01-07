
--[[
parse the stand c struct define to hffi.
]]--

--mock code
local function newContext(c)
	local self = c or {};
	local str = "";
	local def_tab = {}
	function self.defineArrayIfNeed(info)
		-- local arr_frame_data = hffi.array(pointer, AV_NUM_DATA_POINTERS)
		local htype = info.defHTypeStr();
		local name = info.defHName()
		if def_tab[name] == nil then
			local def_str = "local "..name.." = ".."hffi.array("..htype..", "..tostring(info.array_ele_count)..");\n"
			def_tab[name] = def_str
			str = str..def_str
		end
	end
	function self.appendLine(l)
		str = str..l.."\n"
	end
	function self.outStr()
		return str;
	end
	return self;
end

local function newCtypeInfo(c)
	local self = c or {};
	-- self.baseTypeStr. pointerLevel, array_ele_count
	function self.defHTypeStr() -- int
		if(self.pointerLevel > 0) then
			return "pointer";
		end
		return self.baseTypeStr;
	end
	
	function self.defHType()
		-- todo
	end
	
	function self.defHName()
		if( info.array_ele_count > 0) then
			return "_"..sef.defHTypeStr().. "_"..tostring(self.array_ele_count)
		end
		return "_"..sef.defHTypeStr()
	end
	return self;
end
local function word2BaseType(word)
	-- like enum(int), uint8_t , int...
	-- may be struct / struct pointer / function ptr.
end
local function parseLine(ctx, line)
	line.skipSpace();
	if line.startsWith("#") then-- #define, #if 
		return nil;
	end
	if line.startsWith("attribute_deprecated") then
		return nil;
	end
	-- note
	if line.startsWith("/") or line.startsWith("*") then
		return nil
	end
	-- all is blank.
	if line.length() == 0 then
		return nil
	end
	local info = newCtypeInfo();	
	
    local baseType = line.nextWord();
	info.baseTypeStr = baseType;
	line.skipSpace();
	local text = line.nextText(); -- text without space.
	
	local name;
	-- int (*get_encode_buffer)(struct AVCodecContext *s, AVPacket *pkt, int flags);
	if text.startsWith("(*") then
		-- TODO function pointer. 'pointer get_encode_buffer'
	   name = text.nextWord();
	   info.baseTypeStr = "pointer"
	   info.pointerLevel = 0;
	else then
	   info.pointerLevel = text.nextCharCount("*")
	   name = text.nextName();
	   local array_ele_count = text.nextArrayElementCount(); -- [i]
		if(array_ele_count > 0) then
			ctx.defineArrayIfNeed(info);
		end
	end	
	end
	
	local o_line = info.defHTypeStr() .. ", \""..name .. "\" ;"
	ctx.appendLine(o_line);
end
--
local self = {};

function self.parseCStruct(str)
	-- todo
end

return self;