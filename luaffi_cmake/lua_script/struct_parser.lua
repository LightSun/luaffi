
--[[
parse the stand c struct define to hffi.
]]--
local hstring = require("core.hstring")
local ints = require("ints")
--mock code
local function newContext(c)
	local self = c or {};
	local str = "";
	local def_tab = {}
	local cur_struct_name;
	--local struct_map = {}
	function self.defineArrayIfNeed(info)
		-- local arr_frame_data = hffi.array(pointer, AV_NUM_DATA_POINTERS)
		local htype = info.defHTypeStr();
		local name = info.defHName()
		if def_tab[name] == nil then
			--todo may be struct-array/array-array (harray?)
			local def_str = "local "..name.." = ".."hffi.array("..htype..", "..tostring(info.array_ele_count)..");\n"
			def_tab[name] = def_str
			str = str..def_str
		end
	end
	function startStruct(name)
		cur_struct_name = name;
		self.appendLine("local _"..name.." = hffi.struct({")
		self.appendLine("no_data = true ;")
		self.appendLine("free_data = false ;")
	end
	
	function endStruct()
		assert(cur_struct_name, "must call startStruct before end.")
		self.appendLine("})")
		cur_struct_name = ""
	end
	
	function self.appendLine(l)
		str = str..l.."\n"
	end
	function self.outStr()
		return str;
	end
	return self;
end

local INFO_FLAG_ENUM = 1;
local INFO_FLAG_STRUCT = 2;
local INFO_FLAG_SIGNED = 4;
local INFO_FLAG_UNSIGNED = 8;

local function newCtypeInfo(c)
	local self = c or {};
	local _flags = 0;
	local _typeStr;
	local _name;
	-- self.baseTypeStr. pointerLevel, array_ele_count
	function self.defHTypeStr() -- int
		if(self.pointerLevel > 0) then
			return "pointer";
		end
		if(_typeStr) then
			return _typeStr;
		end
		-- may be enum
		if( self.hasFlags(INFO_FLAG_ENUM) == true) then
			_typeStr = "int";
			return _typeStr
		end
		--struct as member
		if(self.hasFlags(INFO_FLAG_STRUCT) == true) then
			-- todo 
		end
		-- has unsigned
		if(self.hasFlags(INFO_FLAG_UNSIGNED) == true) then
			-- todo 
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
	
	function self.addFlags(flags)
		_flags = ints.addFlags(_flags, flags)
		return _flags;
	end
	
	function self.deleteFlags(flags)
		_flags = ints.deleteFlags(_flags, flags)
		return _flags;
	end
	
	function self.hasFlags(flags)
		return ints.hasFlags(_flags, flags);
	end
	
	function self.setName(name)
		_name = name;
	end
	
	function self.getName()
		return _name;
	end
	
	return self;
end
local function word2BaseType(word, flags)
	-- like enum(int), uint8_t , int...
	-- may be struct / struct pointer / function ptr.
end
local function parseLine(ctx, line)
	print("start parse struct line: ", line)
	--todo handle 'typedef struct AVCodecContext {' and '}'
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
	-- start parse
	-- check if is 'typedef struct AVCodecContext {' or 'struct AVCodecContext {'
	line.save()
	local tab_text = line.nextTexts(3)
	if(#tab_text == 3) then
		if tab_text[1] == 'struct' and tab_text[3] == '{' then
			ctx.startStruct(tab_text[2])
			return nil
		end
		if tab_text[1] == 'typedef' and tab_text[2] == 'struct' then
			if line.nextText() == '{' then
				ctx.startStruct(tab_text[3])
				return nil;
			end
		end
	elseif (#tab_text == 1) -- '}'
		if( tab_text[1] == "}") then
			ctx.endStruct();
			return nil;
		end
	end
	line.restore();
	
	-- start parse member line.
    local baseTypeStr = line.nextWord();
	if(baseTypeStr == "const") then
		line.skipSpace();
		baseTypeStr = line.nextWord();
	end
	if(baseTypeStr == "enum") then
		info.addFlags(INFO_FLAG_ENUM)
		line.skipSpace();
		baseTypeStr = line.nextWord();
	end
	if(baseTypeStr == "struct") then
		info.addFlags(INFO_FLAG_STRUCT)
		line.skipSpace();
		baseTypeStr = line.nextWord();
	end
	info.baseTypeStr = baseTypeStr;
	line.skipSpace();
	local text = line.nextText(); -- text without space.
	
	local name;
	-- int (*get_encode_buffer)(struct AVCodecContext *s, AVPacket *pkt, int flags);
	if text.startsWith("(*") then
		-- TODO function pointer. 'pointer get_encode_buffer'
	   name = text.nextWord(")");
	   info.baseTypeStr = "pointer"
	   info.pointerLevel = 0;
	   info.setName(name)
	else 
	   info.pointerLevel = text.nextCharCount("*")
	   name = text.nextName();
	   info.setName(name)
	   local array_ele_count = text.nextArrayElementCount(); -- [i]
		if(array_ele_count > 0) then
			ctx.defineArrayIfNeed(info);
		end	
	end
	
	local o_line = info.defHTypeStr() .. ", \""..name .. "\" ;"
	ctx.appendLine(o_line);
end
--
local self = {};

--str: the struct desc from C.
function self.convertStruct(str)
	-- todo
	local file = io.open(str)
	if not file then
		return nil;
	end

	local ctx = newContext()
	for line in file:lines() do
		parseLine(ctx, hstring.newHString(line))    
	end
	io.close(file)
	-- todo ctx.outStr.
end

return self;