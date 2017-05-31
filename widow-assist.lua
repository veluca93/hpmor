-- Use method aliases for better performance
local nodecopy = node.copy
local nodecopylist = node.copy_list
local nodetail = node.tail
local nodeinsertbefore = node.insert_before
local nodeinsertafter = node.insert_after
local nodetraverseid = node.traverse_id

-- Get node ids from their names
local HLIST = node.id("hlist")
local WHAT = node.id("whatsit")
local COL = node.subtype("pdf_colorstack")

-- Make nodes for beginning and end of colored regions
local color_push = node.new(WHAT,COL)
local color_pop = node.new(WHAT,COL)
color_push.stack = 0
color_pop.stack = 0
color_push.data = "1 0 0 rg" -- PDF code for RGB red
color_push.command = 1
color_pop.command = 2

-- Set to true when the next post-linebreak filter should color the last line
local ColorLastLine = false

-- Function to color the last line in the given list
local color_last_line = function (n)
    -- Get the last hlist in the given list
    local lastLine
    for line in nodetraverseid(HLIST, n) do
        lastLine = line
    end

    -- Surround it with color start/stop
    lastLine.head = nodeinsertbefore(lastLine.head, lastLine.head, nodecopy(color_push))
    nodeinsertafter(lastLine.head, nodetail(lastLine.head), nodecopy(color_pop))
end

-- Callback to check if decreasing the looseness would decrease the line count
local pre_linebreak_test_looseness = function (head, groupeCode)
    -- Build a copy of the paragraph with decreased looseness
    local nM1, iM1 = tex.linebreak(nodecopylist(head), {looseness=tex.looseness-1})

    -- Build a copy of the paragraph normally
    local n, i = tex.linebreak(nodecopylist(head))

    -- Store whether decreasing the looseness does decrease the line count, for "post" callback
    ColorLastLine = iM1.prevgraf < i.prevgraf

    return true
end

-- Callback to colorize the last line of the paragraph when ColorLastLine is true
local post_linebreak_color_last_line = function (head, groupcode)
    if ColorLastLine then
        color_last_line(head)
    end
    return true
end

-- Register callbacks
luatexbase.add_to_callback("pre_linebreak_filter", pre_linebreak_test_looseness, "pre_linebreak_test_looseness")
luatexbase.add_to_callback("post_linebreak_filter", post_linebreak_color_last_line, "post_linebreak_color_last_line")
