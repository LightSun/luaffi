
local m = {}

function m.replaceAll(str, pattern, repl, count)
    local re, _= string.gsub(str, pattern, repl, count)
    return re;
end

function m.trim(str)
    if str == nil then
        return nil, "the string parameter is nil"
    end
    str = string.gsub(str, " ", "")
    return str
end

function m.count(str, substr, from, to)
    if str == nil or substr == nil then
        return nil, "the string or the sub-string parameter is nil"
    end
    from = from or 1
    if to == nil or to > string.len(str) then
        to = string.len(str)
    end
    local _, n = string.gsub(str, substr, '')
    return n
end
function m.startsWith(str, substr)
    if str == nil or substr == nil then
        return nil, "the string or the sub-stirng parameter is nil"
    end
    if string.find(str, substr) ~= 1 then
        return nil
    else
        return true
    end
end

function m.endsWith(str, substr)
    if str == nil or substr == nil then
        return nil, "the string or the sub-string parameter is nil"
    end
    local str_tmp = string.reverse(str)
    local substr_tmp = string.reverse(substr)
    if string.find(str_tmp, substr_tmp) ~= 1 then
        return nil
    else
        return true
    end
end

 function m.split(str, delimeter)
    local find, sub, insert = string.find, string.sub, table.insert
    local res = {}
    local start, start_pos, end_pos = 1, 1, 1
    while true do
        start_pos, end_pos = find(str, delimeter, start, true)
        if not start_pos then
            break
        end
        insert(res, sub(str, start, start_pos - 1))
        start = end_pos + 1
    end
    insert(res, sub(str,start))
    return res
end

function m.isAbsolutePath(path)
    -- unix absolute file path
    if(m.startsWith(path, "/")) then
        return true;
    end
    -- windows disk
    if(string.sub(path, 2, 2) == ":")then
        return true;
    end
    return nil;
end

function m.bytes2Str(bytes)
    --string.char(unpack(bytes))
    local str = ""
    for i = 1 , #bytes do
        str = str..string.char(bytes[i])
    end
    return str;
end

-- 判断第一个字符是否为大写
function m.isUpper(ch)
    local val = ch;
    if(type(ch) == "string") then
        val = string.byte(ch);
    end
    return val >= string.byte('A') and val <= string.byte('Z')
end

function m.isNumber(s)
    if not s then
        return nil
    end
    if string.byte(s) >= 48 and string.byte(s) <= 57 then
        return true;
    end
    return nil;
end

-- pos: start from 1
function m.charAt(str, pos)
    if(pos > #str) then
        return nil;
    end
    return string.byte(string.sub(str, pos, pos))
end

function m.charStrAt(str, pos)
    if(pos > #str) then
        return nil;
    end
    -- string.sub:  first is str, second is start-pos(include), third is end-pos (include)
    return string.sub(str, pos, pos)
end
--print(m.isUpper('n'), m.isUpper('Y'))

return m;