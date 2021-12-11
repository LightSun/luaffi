--[[
band: &&
bnot: ! (1->0, 0->1)
bor: | (or)
bxor:   same to 0, diff to 1(diff-or)
rshift:  >>
lshift: <<
]]--

local ints = {}

function ints.bor(str, op)
	local a = {}
	local tmp;
	for i = 1, #str, 1 do
		--print("i = ", i)	
		tmp = string.sub(str, i, i + 1);	
		if tonumber(tmp)~=nil then
			tmp = tonumber(tmp);
		else 	
			tmp = string.byte(tmp)
		end		
		a[i] = op(i, tmp)
	end
	return bit32.bor(table.unpack(a)); 
end

-- 'EOF '= 'E' | ('O' << 8) | ('F' << 16) | (' ' << 24)
function ints.shift_bor(str)
	return ints.bor(str, function(i, b)
		if(i == 1) then
			return b
		end
		return bit32.lshift(b, (i - 1) * 8) -- <<
	end);
end

-- 'EOF '= 'E' | ('O' << 8) | ('F' << 16) | ((unsigned int)' ' << 24)
function ints.shift_bor2(str)
	return ints.bor(str, function(i, b)
		if(i ==4 and b < 0) then
			b = 0;
		end
		if(i == 1) then
			return b
		end
		return bit32.lshift(b, (i - 1) * 8) -- <<
	end);
end

return ints;
