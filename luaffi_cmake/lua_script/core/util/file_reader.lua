
local m = {};

function m.open(file_in)
    assert(file_in, "file must not be empty")
    local self = {};
    local file;
    local m_lineNum = 1;
    local tab_lines;
   -- local tab_insert_files;
   -- local tab_insert_lines; -- <k,v> key = file, value = tab_lines

    -- func_predicate: return true. means stop skip.
    function self.skipLineUntil(func_predicate, func_collect)
        local line;
        local lineNum;
        while(true) do
            lineNum = m_lineNum;
            line = self.nextLine()
            if not line or func_predicate(lineNum, line) == true then
                break;
            else
                if func_collect then
                    func_collect(lineNum, line);
                end
            end
        end
    end

    function self.readLines()
        local lines = {}
        local line;
        while(true) do
            line = self.nextLine();
            if(not line) then
                break;
            end
            table.insert(lines, line)
        end
        return lines;
    end

    function self.appendLine(line)
        if not tab_lines then
            tab_lines = {};
        end
        table.insert(tab_lines, line);
    end

    function self.appendLines(lines)
        if #lines == 0 then
            return
        end
        if not tab_lines then
            tab_lines = {};
        end
        -- `a2[t]`,`··· = a1[f]`,`···,a1[e]`.
        -- table.move(a1, f, e, t, a2)
        local old_size = #tab_lines
        table.move(lines, 1, #lines, #tab_lines + 1, tab_lines);
        assert(#tab_lines == (old_size + #lines))
        --for j = 1, #tab_lines do
        --    print("after appendLines : "..tab_lines[j])
        --end
    end

    function self.nextLine()
        -- when line is append from outside. we may lineNum not changed.
        -- print("----------------- next line -----------")
        if tab_lines and #tab_lines > 0 then
            return table.remove(tab_lines, 1);
        end
        assert(file, "file is not open or had been closed.")
        local line = file:read("l") -- read a line
        m_lineNum = m_lineNum + 1
        return line;
    end

    function self.getLineNumber()
        return m_lineNum;
    end

    --- receiver: can be function or object.
    function self.stream(receiver)
        assert(file, "file is not open or had been closed.")
        if receiver and type(receiver) ~= 'function' and type(receiver.onStart) == "function" then
            receiver.onStart()
        end
        local fun_process;
        if type(receiver) == 'function' then
            fun_process = receiver;
        else
            fun_process = receiver.processLine;
        end
        assert(fun_process)

        local line;
        local lineNum0;
        while(true) do
            lineNum0 = m_lineNum;
            line = self.nextLine()
            if(line) then
                -- if true, break read . and end
                if fun_process(self, lineNum0, line) then
                    break;
                end
            else
               -- print("line is invalid.")
                break;
            end
        end
        self.close();
        if receiver and type(receiver) ~= 'function'  and type(receiver.onEnd) == "function" then
            receiver.onEnd()
        end
    end
    function self.close()
        if(file) then
            io.close(file)
            file = nil
        end
    end
    function self.open()
        assert(not file, "file had already opened.")
        file = io.open(file_in)
        if not file then
            return nil;
        end
        m_lineNum = 1;
    end
    self.open()
    return self;
end

return m;
