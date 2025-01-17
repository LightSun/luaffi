
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
local strings = require("core.util.strings")
local file_reader = require("core.util.file_reader")
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
local function getWorkDir()
	local info = debug.getinfo(1, "S") -- 第二个参数 "S" 表示仅返回 source,short_src等字段， 其他还可以 "n", "f", "I", "L"等 返回不同的字段信息
	--for k,v in pairs(info) do
	--	print(k, ":", v)
	--end
	local path = info.source
	path = string.sub(path, 2, -1) -- 去掉开头的"@"
	path = string.match(path, "^.*/") -- 捕获最后一个 "/" 之前的部分 就是我们最终要的目录部分
	if(DEBUG) then
		print("work_dir =", path)
	end
	return path;
end
local function newContext(c)
	local self = c or {};
	local str = "";
	local tab_structs = {}	 -- struct names
	local cur_struct_name;	 -- current struct name
	local tab_strs = {};	 -- the all strs of struct-defines.
	local tab_defines = {};
	local _work_dir = getWorkDir(); -- the work dir of current parser.
	local _cur_file; 				  -- the current file to parse
	local _parsed_files = {};
	local _local_defines ;  -- the local defines, like 'local _int_8 = hffi.arrays(int, {8})'

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

	function self.get(s)
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
	function self.printDefines()
		if not tab_defines then
			return;
		end
		print("------- Defines start ------")
		for key, value in pairs(tab_defines) do
			print(key, tostring(value))
		end
		print("------- Defines end ------")
	end
	----------------- local defines. used to avoid redefine ---------
	function self.isLocalDefined(name)
		if(_local_defines and _local_defines[name]) then
			return true;
		end
		return nil;
	end
	function self.addLocalDefine(name)
		if(not _local_defines) then
			_local_defines = {};
		end
		_local_defines[name] = true;
	end
	------------------------------------
	function self.setCurrentFile(file_)
		local targetFile = _work_dir..file_;
		--print("setCurrentFile >>> file_:", file_)
		--print("setCurrentFile >>> targetFile:", targetFile)
		for i = 1, #_parsed_files do
			if(_parsed_files[i] == targetFile) then
				return nil
			end
		end
		_cur_file = file_;
		table.insert(_parsed_files, targetFile)
		return true;
	end
	function self.isFileParsed(_file)
		for i = 1, #_parsed_files do
			if(_parsed_files[i] == _file) then
				return true
			end
		end
		return nil;
	end
	function self.addParsedFile(_filename)
		table.insert(_parsed_files, _filename)
	end
	-----------------------------------------------
	-- return the file path which can be used for parser
	function self.filePath(name)
		if(strings.isAbsolutePath(name)) then
			return name;
		end
		local dir;
		if(strings.isAbsolutePath(_cur_file)) then
			dir = string.match(_cur_file, "^.*/");
		else
			-- '_cur_file' is relative file
			if(string.find(_cur_file, "/")) then
				dir = _work_dir..string.match(_cur_file, "^.*/");
			else
				dir = _work_dir;
			end
		end
		-- the count to reverse
		local count = strings.count(name, "../")
		for i = 1, count do
			dir = string.match(dir, "^.*/");
		end
		return dir..name;
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
	local tab_array_count = {};
	local _pointerLevel = 0;
	-- self.baseTypeStr. pointerLevel, array_ele_count
	function self.setBaseTypeStr(t)
		self.baseTypeStr = t
	end

	function self.processType(target_name)
		local t_name = target_name or _name;
		local t = self.baseTypeStr;
		-- process default . like 'signed age;' or 'unsigned age;'
		if(not t) then
			if (self.hasFlags(INFO_FLAG_UNSIGNED) == true or self.hasFlags(INFO_FLAG_SIGNED) == true) then
				self.baseTypeStr = "int";
				t = "int";
			end
		end

		if (self.hasFlags(INFO_FLAG_UNSIGNED) == true) then
			local ct = tab_types[t]
			if ct then
				self.baseTypeStr = ct;
				print(string.format("setBaseTypeStr >>> cast unsigned type. from %s to %s", t, ct))
			end
		elseif (self.hasFlags(INFO_FLAG_ENUM) == true and t ~= 'pointer') then
			self.baseTypeStr = "int"; -- for c . enum default int.
		end
		-- check array
		if(#tab_array_count == 0) then
			if( self.hasFlags(INFO_FLAG_STRUCT) == true and t ~= 'pointer')then
				if _ctx.hasStruct(t) == false then
					error(string.format("you must parse struct '%s' before %s.", t, _ctx.getCurStructName()))
				else
					self.baseTypeStr = _ctx.getStructObjName(t)..".copy()";
				end
			elseif ctx.hasStruct(t) then
				self.baseTypeStr = _ctx.getStructObjName(t)..".copy()";
			end
			if(_pointerLevel > 0) then
				_ctx.appendLine(string.format("%s, \"%s\";", "pointer", t_name))
			else
				local alias = _ctx.get(self.baseTypeStr);
				local name = alias or self.baseTypeStr;
				_ctx.appendLine(string.format("%s, \"%s\";", name, t_name))
			end
		else
			-- build array desc
			-- like: local arr_frame_data = hffi.arrays(int, {3, 2, 5})
			local desc = "{";
			local len = #tab_array_count;
			for i = 1, len do
				desc = desc .. tostring(tab_array_count[i]);
				if(i ~= len) then
					desc = desc .. ", ";
				end
			end
			desc = desc .. "}";
			-- check pointer type
			if(_pointerLevel > 0) then
				self.baseTypeStr = "pointer";
			end

			local arr_def_name = self.getArrayDefName();
			--todo currently only support one-level-array. support 2-leve/3level ?
			if( self.hasFlags(INFO_FLAG_STRUCT) == true and _pointerLevel == 0) then
				if _ctx.hasStruct(t) == false then
					error(string.format("you must parse struct '%s' before %s.", t, _ctx.getCurStructName()))
				else
					-- local arr_frame_data = hffi.array(pointer, AV_NUM_DATA_POINTERS)
					-- local arr_frame_data = hffi.array({}, {})
					-- isLocalDefined
					if(not _ctx.isLocalDefined(arr_def_name)) then
						_ctx.addLocalDefine(arr_def_name)
						_ctx.appendLine(string.format("local %s = hffi.arrays(%s, %s);",
								arr_def_name, _ctx.getStructObjName(t)..".copy()", desc), true);
					end
				end
			else
				if(not _ctx.isLocalDefined(arr_def_name)) then
					_ctx.addLocalDefine(arr_def_name)
					_ctx.appendLine(string.format("local %s = hffi.arrays(%s, %s);",
							arr_def_name, self.baseTypeStr, desc), true);
				end
			end
			_ctx.appendLine(string.format("%s.copy(), \"%s\";", arr_def_name, t_name))
		end
	end
    function self.appendArrayElementCount(count)
		table.insert(tab_array_count, count);
	end
	function self.setPointerLevel(level)
		_pointerLevel = level;
	end

	function self.getArrayDefName()
		local desc = "";
		local len = #tab_array_count;
		for i = 1, len do
			desc = desc.."_"..tostring(tab_array_count[i]);
		end
		desc = desc .. "";
		return "_"..self.baseTypeStr..desc
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

local BLOCK_TYPE_IF     = 1
local BLOCK_TYPE_ELSEIF = 2
local BLOCK_TYPE_ELSE   = 3
-- local BLOCK_TYPE_ENDIF  = 4
local function newBlockInfo(_type, expre)
	print(string.format(">> newBlockInfo: type = %d, expre = %s", _type, expre))
	local self = {}
	local tab_lineNums = {}
	local tab_lines = {}
	local type0 = _type;
	local _expre = expre;

	function self.setLineNums(line_nums)
		tab_lineNums = line_nums;
	end
	function self.setLines(_lines)
		tab_lines = _lines;
	end
	function self.getType()
		return type0;
	end
	function self.getExpre()
		return _expre;
	end
	function self.getLines()
		return tab_lines;
	end
	return self;
end
--[[
#if FF_API_UNSANITIZED_BITRATES
    int max_bitrate;
#elseif

#else
    int64_t max_bitrate;
#endif
]]
local function handle_ifelse_expre(ctx, hs_line, reader)
	print(" handle_ifelse_expre >> ", hs_line.rawString())
	hs_line.skipText() -- skip '#if'
	hs_line.skipSpace();
	local hs = hstring.newHString("---") -- just holder
	local name = hs_line.nextText()
	local blocks = {}
	local bi = newBlockInfo(BLOCK_TYPE_IF, name)
	-- tmp table
	local tab_lineNums = {};
	local tab_lines = {};
	local func_collect = function(lineNum, line1)
		if(not strings.startsWith(line1, "#")) then
			table.insert(tab_lineNums, lineNum)
			table.insert(tab_lines, line1)
		end
	end
	local func_block_type = function(_lineStr, block_type)
		-- set to info and save to blocks
		bi.setLineNums(tab_lineNums)
		bi.setLines(tab_lines)
		table.insert(blocks, bi)
		-- check if need next block
		if block_type then
			hs.reset(_lineStr)
			hs.skipText(1)
			hs.skipSpace();
			bi = newBlockInfo(block_type, hs.nextText())
			tab_lineNums = {}
			tab_lines = {}
		end
	end
	-- parse case blocks
	reader.skipLineUntil(function(_, line1)
		if(strings.startsWith(line1, "#elseif")) then
			func_block_type(line1, BLOCK_TYPE_ELSEIF)
		elseif (strings.startsWith(line1, "#else")) then
			func_block_type(line1, BLOCK_TYPE_ELSE)
		elseif strings.startsWith(line1, "#endif") then
			func_block_type(nil, nil)
			return true;
		end
	end, func_collect)
	-- handle blocks
	for i = 1, #blocks do
		bi = blocks[i]
		-- if 'bi.getExpre() = nil'

		if(DEBUG) then
			if(bi.getType() == BLOCK_TYPE_IF) then
				print("start handle:  if block =", bi.getExpre(), #bi.getLines())
			elseif 	bi.getType() == BLOCK_TYPE_ELSEIF then
				print("start handle:  elseif block =", bi.getExpre(), #bi.getLines())
			elseif 	bi.getType() == BLOCK_TYPE_ELSE then
				print("start handle:  else block =", bi.getExpre(), #bi.getLines())
			end
		end
		-- else
		if	bi.getType() == BLOCK_TYPE_ELSE then
			print("find valid else: ", bi.getExpre())
			reader.appendLines(bi.getLines(), true);
			return;
		end
		-- if/elseif
		local num = tonumber(bi.getExpre())
		if( num and num ~= 0 ) then
			print("find valid number: ", bi.getExpre())
			for j = 1, #bi.getLines() do
				print("__line="..bi.getLines()[j])
			end
			reader.appendLines(bi.getLines(), true);
			return;
		end
		if(ctx.defined(bi.getExpre()) and ctx.get(bi.getExpre()) ~= 0) then
			print("find valid expre: ", bi.getExpre())
			reader.appendLines(bi.getLines(), true);
			return;
		end
	end
	if(DEBUG) then
		print("no if-elseif-else block valid.")
	end
end
--[[
handle #define and typedef
member can be: unsigned/unsigned <base_type>
			 struct member
			 pointer/struct pointer/function pointer
			 array/struct-array. may be multi level
]]--
local function parseLine(reader, ctx, line, lineNum)
	print(string.format("start parse struct line -- %d: %s", lineNum, line.toString()))
	-- handle 'typedef struct AVCodecContext {' and '}'
	line.skipSpace();
	if line.startsWith("attribute_deprecated") then
		line.skipText()
		line.skipSpace()
		if(line.isEnd()) then
			return nil;
		end
	end
	--todo  support '// and /*.../'
	if line.startsWith("/") or line.startsWith("*") then
		return nil
	end
	-- all is blank.
	if line.length() == 0 then
		return nil
	end
	-- handle "#include"
	if(line.startsWith("#include")) then
		line.skipText();
		line.skipSpace()
		-- get file name
		local filename = ctx.filePath(line.nextQuoteText());
		print("filename: ", filename)
		-- already parsed?
		if(ctx.isFileParsed(filename)) then
			return nil;
		end
		ctx.addParsedFile(filename);
		local reader0 = file_reader.open(filename)
		local include_lines = reader0.readLines()
		reader0.close()
		reader.appendLines(include_lines)
		return nil;
	end

	-- handle '#define'
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
			ctx.define(word, tonumber(val) or val)
		end
	end
	-- handle if-elseif-else
	if(line.startsWith("#if")) then
		handle_ifelse_expre(ctx, line, reader)
		return nil;
	end
	-- TODO other?
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
		baseTypeStr = line.nextWord(";*");
		-- check may be 'signed age;'
		line.save();
		line.skipSpace()
		if(line.startsWith(";")) then
			-- no assigned type. default is int in 'C'.
			info.setBaseTypeStr("int")
			info.setName(baseTypeStr)
			info.processType()
			return true;
		else
			line.restore();
		end
	elseif (baseTypeStr == "unsigned") then
		info.addFlags(INFO_FLAG_UNSIGNED)
		line.skipSpace();
		baseTypeStr = line.nextWord(";*");
		-- check may be 'unsigned age;'
		line.save();
		line.skipSpace()
		if(line.startsWith(";")) then
			-- no assigned type. default is int in 'C'.
			info.setBaseTypeStr("int")
			info.setName(baseTypeStr)
			info.processType()
			return true;
		else
			line.restore();
		end
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

		--
		line.skipUntilChar("(")
		line.skip(1) -- '('
		if( line.skipUntilChar(")") > 0 ) then
			-- found
		else
			-- not found. ')' must be in next lines.
			while(true) do
				local line_str = reader.nextLine();
				line.reset(line_str)
				if(line.skipUntilChar(")") >= 0 ) then
					break; -- found.
				end
			end
		end
		-- for function pointer we should remove flags of enum and struct.
		-- eg: enum AVPixelFormat (*get_format)(struct AVCodecContext *s, const enum AVPixelFormat * fmt);
	else
	    info.setPointerLevel(line.nextCharCount("*"))
	    name = line.nextName(",;[");
	    info.setName(name)
		if(DEBUG) then
			print(string.format("after name left: '%s'",line.toString()))
		end
		local array_ele_count;
		while(true) do
			array_ele_count = line.nextArrayElementCount(ctx); -- [i]
			if(array_ele_count > 0) then
				info.appendArrayElementCount(array_ele_count)
			else
				break;
			end
		end
		-- skip ';'
		if(line.startsWith(";")) then
			info.processType();
			return true
		end
		-- may have '//xxx.xxx'
		--todo handle '/* ... */'
		line.skipSpace()
		if(line.startsWith("//")) then
			info.processType();
			return true
		end
		-- when not array. may be ' int width, height;'
		if(not array_ele_count or array_ele_count == 0) then
			print(" >>> not array, check 'int width, height;'.")
			if( line.skipUntilChar(",") >= 0 ) then
				-- found
				line.skip(1) -- skip ','
				line.skipSpace();
				name = line.nextName(";[");
				-- process 'int width, height;'
				info.processType();
				info.processType(name)
				return true;
			end
		end
	end
	info.processType();
	return true
end
--
local self = {};

--str: the struct desc from C.
function self.convertStruct(str, _ctx)

	local ctx = _ctx or newContext()
	-- if already parsed ?
	if( not ctx.setCurrentFile(str) ) then
		return
	end
	--ctx.filePath("abc.in")
	local reader = file_reader.open(str)
	reader.stream(function(r, lineNum, line)
		parseLine(reader, ctx, hstring.newHString(line), lineNum)
	end)
	if not _ctx then
		print("-------- convertStruct result ---------- ")
		ctx.printDefines();
		print(ctx.outStr())
	end
end

--self.convertStruct("test_res/struct1.in")
--self.convertStruct("test_res/struct_if.in")

--self.convertStruct("test_res/ffmpeg.in")
--self.convertStruct("test_res/ffmpeg_avstream.in")
--self.convertStruct("test_res/struct_func_multi_lines.in")
--self.convertStruct("test_res/struct_simple.in")
--self.convertStruct("test_res/struct_simple2.in")

local function test_ffmpeg()
	local ctx = newContext()
	self.convertStruct("test_res/ffmpeg_avcodec.in", ctx)
	--self.convertStruct("test_res/ffmpeg_avcodec_ctx.in", ctx)
	--self.convertStruct("test_res/ffmpeg_avformat_ctx.in", ctx)
	self.convertStruct("test_res/ffmpeg_avframe.in", ctx)

	print("-------- convertStruct result ---------- ")
	ctx.printDefines();
	print(ctx.outStr())
end
test_ffmpeg();

return self;