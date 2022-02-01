
local strings = require "core.util.strings"

local m = {}

function m.newHString(str)
	assert(type(str) == 'string', "for newHString >>> must be lua string.")
	local self = {}
	local _str = str;
	local pos = 1 -- start with 1
	local tab_savePos;

	--- return -1, means not found any char. 0 means the first char matches. or else found
	---
	function self.skipUntilChar(chars)
		local tab_chs = {};
		if( chars and type(chars) == "string" )then
			for i = 1, #chars do
				tab_chs[strings.charAt(chars, i)] = true
			end
		end
		local code;
		local count = -1;
		for i = pos, #_str do
			code = strings.charAt(_str, i)
			if(tab_chs[code]) then
				count = i - pos;
				pos = pos + count;
				break;
			end
		end
		return count;
	end
	
	function self.skipSpace()
		-- need  '\r\n\t\v\f'?
		local space_code = string.byte(" ");
		local newline_code = string.byte("\n");
		local t_code = string.byte("\t");
		local code;
		for i = pos, #_str do
			code = strings.charAt(_str, i)
			if(code ~= space_code and code ~= newline_code and code ~= t_code) then
				break;
			end
			pos = pos + 1;
		end
	end	
	
	function self.skip(count)
		pos = pos + count
	end

	function self.skipWord(c, except_chars)
		if not c then
			c = 1
		else
			assert(type(c) == "number")
		end
		for i = 1, c do
			self.nextWord(except_chars)
			if(i ~= c) then
				self.skipSpace()
			end
		end
	end

	function self.skipText(c, except_chars)
		if not c then
			c = 1
		else
			assert(type(c) == "number")
		end
		for i = 1, c do
			self.nextText(except_chars)
			if(i ~= c) then
				self.skipSpace()
			end
		end
	end
		
	function self.requireChar(ch)
		assert(type(ch) == 'string')
		assert(strings.charAt(_str, pos) == string.byte(ch), "requireChar '"..ch.."'!")
		return true;
	end
	
	function self.startsWith(s)
		assert(type(s) == "string")
		if #s > #_str - pos + 1 then
			return nil
		end
		local si = 1
		for i = pos, #_str do
			if(string.byte(string.sub(_str, i, i)) ~= string.byte(string.sub(s, si, si)) ) then
				return nil;
			end
			si = si + 1
			if si > #s then
				break;
			end
		end
		return true;
	end

	function self.endsWith(s)
		assert(type(s) == "string")
		local si = #s;
		for i = #_str, pos, -1 do
			if(string.byte(string.sub(_str, i, i)) ~= string.byte(string.sub(s, si, si)) ) then
				return nil;
			end
			si = si - 1;
			if(si < 1) then
				break;
			end
		end
	end
	
	function self.length()
		return #_str - pos + 1;
	end

	--string.find: return the count of regex
	function self.nextWord(except_chars)
		-- must start with letter. $_
		self.save()
		local res = self.nextText(except_chars);
		if res then
			local pat =  "^[%a$_][%a%d$_]*"
			local pr = string.match(res, pat);
			if(pr == nil) then
				self.restore()
				print(string.format("called nextWord():  has text '%s', but not valid word text.", res))
				return nil;
			end
			self.pop()
			return res;
		end
		self.restore()
		return nil;
	end
	
	function self.nextText(except_chars)
		-- the except table which contains not allow chars.
		local except_tab = {};
		if( except_chars and type(except_chars) == "string" )then
			for i = 1, #except_chars do 
				except_tab[strings.charAt(except_chars, i)] = true
			end
		end
		
		local space_code = string.byte(" ");
		local newline_code = string.byte("\n");
		local t_code = string.byte("\t");
		
		local code;
		local count = 0;
		for i = pos, #_str do
			code = strings.charAt(_str, i)
			if(code == space_code or code == newline_code or code == t_code) then
				break;
			end
			if except_tab[code] then
				break;
			end
			count  = count + 1 ;
		end
		if count == 0 then
			return nil
		end
		-- print("nextText: count  = ", count)
		local res = string.sub(_str, pos, pos + count  - 1)
		pos = pos + count;
		return res;
	end
	
	function self.nextWords(count, except_chars)
		local tab = {}
		local tmp;
		for i = 1, count do
			tmp = self.nextWord(except_chars);
			-- print("tmp = ", tmp)
			if not tmp then
				return tab
			end
			-- success , we need skip space
			self.skipSpace();
			table.insert(tab, tmp)
		end
		return tab;
	end

	-- except_chars: optional
	function self.nextTexts(count, except_chars)
		local tab = {}
		local tmp;
		for i = 1, count do
			tmp = self.nextText(except_chars);
			if not tmp then
				return tab
			end
			self.skipSpace();
			table.insert(tab, tmp)
		end
		return tab;
	end
	
	function self.nextName(except_chars)
		return self.nextText(except_chars);
	end
	
	function self.nextCharCount(ch)
		assert(type(ch) == 'string')
		local c = string.byte(ch);
		local count = 0;
		for i = pos, #_str do 
			if strings.charAt(_str, i) ~= c then
				break;
			end
			count = count + 1
		end
		pos = pos + count;
		return count;
	end
	
	function self.nextChar()
		local res = strings.charAt(_str, pos)
		if res then
			pos = pos + 1;
		end
		return res
	end
	
	function self.nextCharStr()
		local res = strings.charStrAt(_str, pos)
		if res then
			pos = pos + 1;
		end
		return res
	end
	
	function self.nextArrayElementCount(ctx)
		--assert(strings.charAt(_str, pos) == string.byte("["), "for nextArrayElementCount. require '[${number}]'")
		if strings.charAt(_str, pos) ~= string.byte("[") then
			return 0;
		end
		local code_end = string.byte("]");
		local count = 0;
		for i = pos + 1, #_str do 
			if strings.charAt(_str, i) == code_end then			
				break;
			end
			count = count + 1 ;
		end
		if(count == 0) then
			return 0;
		end
		local numStr = string.sub(_str, pos + 1, pos + count)
		print("numStr >>> ", numStr)
		assert(strings.count(numStr, ".") ~= 0, "must be integer")
		local num = tonumber(numStr)
		-- may be from '#define a 3'
		if( num == nil) then
			num = ctx.getInt(numStr)
			if(num == nil) then
				error("nextArrayElementCount>>> you must define the int value to the context for '"..numStr.."'")
			end
		end
		-- skip count with '[' and ']'
		pos = pos + 2 + count
		return num;
	end
	-- "abcd" -> abcd
	function self.nextQuoteText()
		if strings.charAt(_str, pos) ~= string.byte("\"") then
			return nil;
		end
		local code_end = string.byte("\"");
		local count = 0;
		for i = pos + 1, #_str do
			if strings.charAt(_str, i) == code_end then
				break;
			end
			count = count + 1 ;
		end
		if(count == 0) then
			return 0;
		end
		local txt = string.sub(_str, pos + 1, pos + count)
		-- skip count with '\"' and '\"'
		pos = pos + 2 + count
		return txt;
	end
	--------------------------
	function self.isEnd()
		return pos == #_str;
	end
	
	function self.save()
		if not tab_savePos then
			tab_savePos = {}
		end
		table.insert(tab_savePos, pos)
		return #tab_savePos
	end
	
	function self.restore(target)
		assert(tab_savePos, "restore must called after save")
		if( not target ) then
			target = #tab_savePos;
		end
		pos = tab_savePos[target]
		table.remove(tab_savePos, target)
	end
	
	function self.reset(__str)
		if __str then
			_str = __str;
		end
		pos = 1;
		tab_savePos = {}
	end

	function self.pop()
		if(tab_savePos) then
			table.remove(tab_savePos, #tab_savePos)
		end
	end

	function self.toString()
		return string.sub(_str, pos)
	end

	function self.rawString()
		return _str;
	end
	
	return self;
end

return m;