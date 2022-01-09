
local bits = {};
--数字转二进制
function bits.ToSecond(num)
    local str = ""
    local tmp = num
    while (tmp > 0) do
        if (tmp % 2 == 1) then
            str = str .. "1"
        else
            str = str .. "0"
        end

        tmp = math.modf(tmp / 2)
    end
    str = string.reverse(str)
    return str
end

--先补齐两个数字的二进制位数
function bits.MakeSameLength(num1, num2, binary)
    local str1;
    local str2;
    if( binary) then
        if(type(num1) == 'string') then
            str1 = num1;
        else
            str1 = tostring(num1)
        end
        if(type(num2) == 'string') then
            str2 = num2;
        else
            str2 = tostring(num2)
        end
    else
        str1 = bits.ToSecond(num1)
        str2 = bits.ToSecond(num2)
    end

    local len1 = string.len(str1)
    local len2 = string.len(str2)
    --print("len1 = " .. len1)
    --print("len2 = " .. len2)

    local len = 0
    local x = 0

    if (len1 > len2) then
        x = len1 - len2
        for i = 1, x do
            str2 = "0" .. str2
        end
        len = len1
        --print("len = "..len)
    elseif (len2 > len1) then
        x = len2 - len1
        for i = 1, x do
            str1 = "0" .. str1
        end
        len = len2
    end
    len = len1
    return str1, str2, len
end

--按位或
function bits.BitOr(num1, num2, binary)
    local str1, str2, len = bits.MakeSameLength(num1, num2, binary)
    local rtmp = ""

    for i = 1, len do
        local st1 = tonumber(string.sub(str1, i, i))
        local st2 = tonumber(string.sub(str2, i, i))

        if(st1 ~= 0) then
            rtmp = rtmp .. "1"
        elseif (st1 == 0) then
            --print("00000")
            if (st2 == 0) then
                rtmp = rtmp .. "0"
            end
        end
    end
    rtmp = tostring(rtmp)
    return rtmp
end

--按位与
function bits.BitAnd(num1, num2, binary)
    local str1, str2, len = bits.MakeSameLength(num1, num2, binary)
    local rtmp = ""

    for i = 1, len do
        local st1 = tonumber(string.sub(str1, i, i))
        local st2 = tonumber(string.sub(str2, i, i))


        if(st1 == 0) then
            rtmp = rtmp .. "0"
        else
            if (st2 ~= 0) then
                rtmp = rtmp .. "1"
            else
                rtmp = rtmp .. "0"
            end
        end
    end
    rtmp = tostring(rtmp)
    return rtmp
end

--按位异或
function bits.BitNotOr(num1, num2, binary)
    local str1, str2, len = bits.MakeSameLength(num1, num2, binary)
    local rtmp = ""

    for i = 1, len do
        local st1 = tonumber(string.sub(str1, i, i))
        local st2 = tonumber(string.sub(str2, i, i))

        if (st1 ~= st2) then
            rtmp = rtmp .. "1"
        else
            rtmp = rtmp .. "0"
        end
    end
    rtmp = tostring(rtmp)
    return rtmp
end

--取反
function bits.BitNot(num)
    local str = bits.ToSecond(num)
    local len = string.len(str)
    local rtmp = ""
    for i = 1, len do
        local st = tonumber(string.sub(str, i, i))
        if (st == 1) then
            rtmp = rtmp .. "0"
        elseif (st == 0) then
            rtmp = rtmp .. "1"
        end
    end
    rtmp = tostring(rtmp)
    return rtmp
end

--jinzhi: 2 means 2 -> 10
function bits.BitToNum10(num)
    return tonumber(num, 2)
end

--[[
print(math.modf(12.34)) -- 12  0.34
local a = 9; -- 1111 . a &= ~8
print("bits.Negate: ", bits.BitNot(8))
print("bits.BitYiOr: ", bits.BitNotOr(8, 4))
print("a &= ~8: ", bits.BitAnd(a, bits.BitToNum10(bits.BitNotOr(8))) )
]]--

return bits;