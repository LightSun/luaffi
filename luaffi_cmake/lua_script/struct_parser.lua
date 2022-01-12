
-- debug
--package.cpath = package.cpath .. ';C:/Users/Administrator/AppData/Roaming/JetBrains/IdeaIC2020.3/plugins/intellij-emmylua/classes/debugger/emmy/windows/x64/?.dll'
--local dbg = require('emmy_core')
--dbg.tcpListen('localhost', 9966)
--dbg.waitIDE()
--dbg.breakHere()

--[[
parse the stand c struct define to hffi.
]]--
local hstring = require("core.hstring")
local strings = require("core.util.Strings")
local ints = require("ints")
--[[
{"sint8", HFFI_TYPE_SINT8},
{"uint8", HFFI_TYPE_UINT8},
{"sint16", HFFI_TYPE_SINT16},
{"uint16", HFFI_TYPE_UINT16},
{"sint32", HFFI_TYPE_SINT32},
{"uint32", HFFI_TYPE_UINT32},
{"sint64", HFFI_TYPE_SINT64},
{"uint64", HFFI_TYPE_UINT64},
{"float", HFFI_TYPE_FLOAT},
{"double", HFFI_TYPE_DOUBLE},

{"void", HFFI_TYPE_VOID},
{"pointer", HFFI_TYPE_POINTER},

{"byte", HFFI_TYPE_SINT8},
{"bool", HFFI_TYPE_SINT8},
{"short", HFFI_TYPE_SINT16},
{"long", HFFI_TYPE_SINT64},
{"int", HFFI_TYPE_INT},
//
{"uint", HFFI_TYPE_UINT32},
{"size_t", HFFI_TYPE_UINT32},
{"int64_t", HFFI_TYPE_SINT64},
{"int32_t", HFFI_TYPE_SINT32},
{"int16_t", HFFI_TYPE_SINT16},
{"int8_t", HFFI_TYPE_SINT8},
{"uint64_t", HFFI_TYPE_UINT64},
{"uint32_t", HFFI_TYPE_UINT32},
{"uint16_t", HFFI_TYPE_UINT16},
{"uint8_t", HFFI_TYPE_UINT8},
]]--
--mock code
local tab_types = {}
tab_types["sint8"] = "uint8"
tab_types["sint16"] = "uint16"
tab_types["sint32"] = "uint32"
tab_types["sint64"] = "uint64"
tab_types["int64_t"] = "uint64"
tab_types["int32_t"] = "uint32"
tab_types["int16_t"] = "uint16"
tab_types["int8_t"] = "uint8"

tab_types["char"] = "uint8"
tab_types["byte"] = "uint8"
tab_types["short"] = "uint16"
tab_types["long"] = "uint64"
tab_types["int"] = "uint32"
--[[
tab_types["sint8"] = sint8
tab_types["sint16"] = sint16
tab_types["sint32"] = sint32
tab_types["sint64"] = sint64
tab_types["float"] = float
tab_types["double"] = double
tab_types["int64_t"] = sint64
tab_types["int32_t"] = sint32
tab_types["int16_t"] = sint16
tab_types["int8_t"] = sint8

tab_types["char"] = sint8
tab_types["byte"] = sint8
tab_types["bool"] = bool
tab_types["short"] = short
tab_types["long"] = long
tab_types["int"] = int


tab_types["void"] = void
tab_types["pointer"] = pointer
tab_types["uint64"] = uint64
tab_types["uint32"] = uint32
tab_types["uint16"] = uint16
tab_types["uint8"] = uint8
tab_types["uint"] = uint32
tab_types["size_t"] = uint32
tab_types["uint64_t"] = uint64
tab_types["uint32_t"] = uint32
tab_types["uint16_t"] = uint16
tab_types["uint8_t"] = uint8
]]--

local DEBUG = 1;

local function newContext(c)
	local self = c or {};
	local str = "";
	local tab_structs = {} -- struct names
	local cur_struct_name; -- current struct name
	local tab_strs = {}; -- the all strs of struct-defines.
	local tab_defines;

	function self.startStruct(name)
		--print("============ add struct: ", name)
		table.insert(tab_structs, name)
		cur_struct_name = name;
		-- #define no_data_name
		self.appendLine("local "..self.getStructObjName(name).." = hffi.struct({")
		if(self.defined(string.format("HAS_DATA_%s", name))) then
			self.appendLine("no_data = false ;")
		else
			self.appendLine("no_data = true ;")  -- default
		end
		if(self.defined(string.format("FREE_DATA_%s", name))) then
			self.appendLine("free_data = true ;")
		else
			self.appendLine("free_data = false ;") -- default
		end
	end

	function self.endStruct()
		assert(cur_struct_name, "must call startStruct before end.")
		self.appendLine("})")
		cur_struct_name = ""
		table.insert(tab_strs, str)
		str = "";
	end

	function self.getStructObjName(s)
		return "_"..s;
	end

	function self.hasStruct(s)
		if not s then
			return nil;
		end
		for i = 1, #tab_structs do
			if tab_structs[i] == s then
				return true;
			end
		end
		return nil;
	end

	function self.getCurStructName()
		return cur_struct_name
	end
	function self.define(var, value)
		if not tab_defines then
			tab_defines = {}
		end
		tab_defines[var] = value;
	end
	function self.defined(s)
		return tab_defines and tab_defines[s] ~= nil
	end
	function self.getInt(s)
		if not tab_defines then
			error(string.format("can't find number for %s", s))
		end
		return tab_defines[s]
	end
	------------------------------------

	function self.appendLine(l, before_cur_struct)
		if( before_cur_struct) then
			str = l.."\n"..str;
		else
			str = str..l.."\n";
		end
	end
	function self.outStr()
		local out_str = "";
		for i = 1, #tab_strs do
			out_str = out_str..tab_strs[i]
		end
		return out_str;
	end
	return self;
end

local INFO_FLAG_ENUM = 1;
local INFO_FLAG_STRUCT = 2;
local INFO_FLAG_SIGNED = 4;
local INFO_FLAG_UNSIGNED = 8;

local function newCtypeInfo(ctx, c)
	assert(ctx, "ctx must not be nil.")
	local self = c or {};
	local _ctx = ctx;
	local _flags = 0;
	local _name;
	local _arr_ele_count = 0;
	local _pointerLevel = 0;
	-- self.baseTypeStr. pointerLevel, array_ele_count
	function self.setBaseTypeStr(t)
		self.baseTypeStr = t
	end

	function self.processType()
		local t = self.baseTypeStr;
		if (self.hasFlags(INFO_FLAG_UNSIGNED) == true) then
			local ct = tab_types[t]
			if ct then
				self.baseTypeStr = ct;
				print(string.format("setBaseTypeStr >>> cast unsigned type. from %s to %s", t, ct))
			end
		elseif (self.hasFlags(INFO_FLAG_ENUM) == true) then
			self.baseTypeStr = "int"; -- for c . enum default int.
		end
		-- check array
		if(_arr_ele_count == 0) then
			if( self.hasFlags(INFO_FLAG_STRUCT) == true) then
				if _ctx.hasStruct(t) == false then
					error(string.format("you must parse struct '%s' before %s.", t, _ctx.getCurStructName()))
				else
					self.baseTypeStr = _ctx.getStructObjName(t)..".copy()";
				end
			end
			if(_pointerLevel > 0) then
				_ctx.appendLine(string.format("%s, %s;", "pointer", _name))
			else
				_ctx.appendLine(string.format("%s, %s;", self.baseTypeStr, _name))
			end
		else
			--todo currently only support one-level-array. support 2-leve/3level ?
			if( self.hasFlags(INFO_FLAG_STRUCT) == true) then
				if _ctx.hasStruct(t) == false then
					error(string.format("you must parse struct '%s' before %s.", t, _ctx.getCurStructName()))
				else
					-- local arr_frame_data = hffi.array(pointer, AV_NUM_DATA_POINTERS)
					-- local arr_frame_data = hffi.array({}, {})
					local desc = "";
					for i = 1, _arr_ele_count do
						desc = desc.._ctx.getStructObjName(t)..".copy()"
						if( i ~= _arr_ele_count) then
							desc = desc..", "
						end
					end
					_ctx.appendLine(string.format("local %s = hffi.array({ %s });",
							self.getArrayDefName(), desc), true);
				end
			else
				_ctx.appendLine(string.format("local %s = hffi.array(%s, %d);",
						self.getArrayDefName(), self.baseTypeStr, _arr_ele_count), true);
			end
			if(_pointerLevel > 0) then
				_ctx.appendLine(string.format("%s, %s;", "pointer", _name))
			else
				_ctx.appendLine(string.format("%s, %s;", self.getArrayDefName(), _name))
			end
		end
	end

	function self.setArrayElementCount(count)
		_arr_ele_count = count;
	end

	function self.setPointerLevel(level)
		_pointerLevel = level;
	end

	function self.getArrayDefName()
		return "_"..self.baseTypeStr.. "_"..tostring(_arr_ele_count)
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

local function evalExpression(ctx, expre)
	--todo latter support more
	return tonumber(expre)
end
--[[
#if FF_API_UNSANITIZED_BITRATES
    int max_bitrate;
#else
    int64_t max_bitrate;
#endif
]]
local function handle_ifelse_expre(ctx, line, func_next_line)

end
--[[
handle #define and typedef
member can be: unsigned/unsigned <base_type>
			 struct member
			 pointer/struct pointer/function pointer
			 array/struct-array. may be multi level
]]--
local function parseLine(ctx, line, lineNum)
	print(string.format("start parse struct line -- %d: %s", lineNum, line.toString()))
	-- handle 'typedef struct AVCodecContext {' and '}'
	line.skipSpace();
	if line.startsWith("attribute_deprecated") then
		return nil;
	end
	--todo  support '// and /*.../'
	if line.startsWith("/") or line.startsWith("*") then
		return nil
	end
	-- all is blank.
	if line.length() == 0 then
		return nil
	end
	if(line.startsWith("#define")) then
		line.skipText();
		line.skipSpace()
		local word = line.nextWord()
		line.skipSpace()
		local val = line.nextText()
		if not val then
			return nil
		else
			-- currently only support number
			ctx.define(word, tonumber(val))
		end
	end
	--todo support 'if elseif else endif'
	if line.startsWith("#") then-- #define, #if
		return nil;
	end
	local info = newCtypeInfo(ctx);
	-- start parse
	-- check if is 'typedef struct AVCodecContext {' or 'struct AVCodecContext {'
	line.save()
	local tab_text = line.nextTexts(3)
	if #tab_text == 0 then
		return nil
	end
	if(#tab_text == 3) then
		if tab_text[1] == 'struct' and tab_text[3] == '{' then
			ctx.startStruct(tab_text[2])
			return nil
		end
		if tab_text[1] == 'typedef' and tab_text[2] == 'struct' then
			if(strings.endsWith(tab_text[3], "{")) then
				ctx.startStruct(string.sub(tab_text[3], 1, -2)); -- -1 is the last. and -2 is the 'last - 1'
				return nil
			end
			line.skipSpace()
			if line.nextText() == '{' then
				ctx.startStruct(tab_text[3])
				return nil;
			end
		end
	else
		--if (#tab_text == 1) then -- '}'
		if( tab_text[1] == "};" or tab_text[1] == "}") then
			ctx.endStruct();
			return nil;
		end
		-- '}S;'
		if string.match(tab_text[1], "^[}][%a$_][%a%d$_]*[;]?") then
			ctx.endStruct();
			return nil;
		end
	end
	line.restore();

	-- start parse member line.
	local baseTypeStr = line.nextWord("*");
	if(baseTypeStr == "typedef") then
		line.skipSpace();
		baseTypeStr = line.nextWord("*");
	end
	if(baseTypeStr == "const") then
		line.skipSpace();
		baseTypeStr = line.nextWord("*");
	end

	if(baseTypeStr == "enum") then
		info.addFlags(INFO_FLAG_ENUM)
		line.skipSpace();
		baseTypeStr = line.nextWord("*");
	elseif (baseTypeStr == "struct") then
		info.addFlags(INFO_FLAG_STRUCT)
		line.skipSpace();
		baseTypeStr = line.nextWord("*");
	elseif (baseTypeStr == "signed") then
		info.addFlags(INFO_FLAG_SIGNED)
		line.skipSpace();
		baseTypeStr = line.nextWord("*");
	elseif (baseTypeStr == "unsigned") then
		info.addFlags(INFO_FLAG_UNSIGNED)
		line.skipSpace();
		baseTypeStr = line.nextWord("*");
	end
	if DEBUG then
		print("before check * >>>  baseTypeStr = ", baseTypeStr)
	end

	info.setBaseTypeStr(baseTypeStr);
	if(line.startsWith("*")) then
		print(string.format(":: base_type convert from %s to pointer", baseTypeStr));
		info.setBaseTypeStr("pointer")
		line.skip(1)
	end
	line.skipSpace();

	if DEBUG then
		print(string.format("after parse base type: '%s'", line.toString()));
	end

	local name;
	-- check function pointer
	-- int (*get_encode_buffer)(struct AVCodecContext *s, AVPacket *pkt, int flags);
	if line.startsWith("(*") then
		--function pointer.
		line.skip(2)
	    name = line.nextWord(")");
	    info.setBaseTypeStr("pointer")
	    info.setName(name)
	else
	    info.setPointerLevel(line.nextCharCount("*"))
	    name = line.nextName(";[");
	    info.setName(name)

		print(string.format("after name left: '%s'",line.toString()))
	    local array_ele_count = line.nextArrayElementCount(ctx); -- [i]
		info.setArrayElementCount(array_ele_count);
	end
	info.processType();
	return true
end
--
local self = {};

--str: the struct desc from C.
function self.convertStruct(str)

	local file = io.open(str)
	if not file then
		return nil;
	end

	local ctx = newContext()
	local line;
	local lineNum = 1;
	while(true) do
		line = file:read("l") -- read a line
		if(line) then
			parseLine(ctx, hstring.newHString(line), lineNum)
			lineNum = lineNum + 1
		else
			break;
		end
	end
--[[ --ok
	for line in file:lines() do
		parseLine(ctx, hstring.newHString(line))
	end
]]
	io.close(file)
	print("-------- convertStruct result: ")
	print(ctx.outStr())
end

self.convertStruct("test_res/struct1.in")

return self;