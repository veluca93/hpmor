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
local color_p1 = node.new(WHAT,COL)
local color_p1s = node.new(WHAT,COL)
local color_m1 = node.new(WHAT,COL)
local color_pm1 = node.new(WHAT,COL)
local color_pm1s = node.new(WHAT,COL)
local color_pop = node.new(WHAT,COL)
color_p1.stack = 0
color_p1.command = 1
color_p1.data = "0 0 1 rg" -- PDF code for RGB blue
color_p1s.stack = 0
color_p1s.command = 1
color_p1s.data = "0 0.7 0.7 rg" -- PDF code for RGB dark cyan
color_m1.stack = 0
color_m1.command = 1
color_m1.data = "1 0 0 rg" -- PDF code for RGB red
color_pm1.stack = 0
color_pm1.command = 1
color_pm1.data = "1 0 1 rg" -- PDF code for RGB magenta
color_pm1s.stack = 0
color_pm1s.command = 1
color_pm1s.data = "1 .5 .5 rg" -- PDF code for RGB pink
color_pop.stack = 0
color_pop.command = 2

-- Color to use for last line in the next post-linebreak filter call (nil = no color)
local LastLineColor = nil

-- Function to color the last line in the given list
local color_last_line = function (n)
    -- Get the last hlist in the given list
    local lastLine
    for line in nodetraverseid(HLIST, n) do
        lastLine = line
    end

    -- Surround it with color start/stop
    lastLine.head = nodeinsertbefore(lastLine.head, lastLine.head, nodecopy(LastLineColor))
    nodeinsertafter(lastLine.head, nodetail(lastLine.head), nodecopy(color_pop))
end

-- Callback to check if changing the looseness by +-1 would affect the line count
local pre_linebreak_test_looseness = function (head, groupeCode)
    -- Disable underfull and overfull boxes reporting
    luatexbase.add_to_callback("hpack_quality", function() end, "hpqfilter")
    luatexbase.add_to_callback("vpack_quality", function() end, "vpqfilter")

    -- Build a copy of the paragraph normally
    local n, i = tex.linebreak(nodecopylist(head))

    -- Build a copy of the paragraph with increased looseness and default emergency stretch
    local nP1s, iP1s = tex.linebreak(nodecopylist(head), {looseness=tex.looseness+1})

    local nP1, iP1
    if iP1s.prevgraf > i.prevgraf then
        -- It worked with the default emergency stretch, let's try without
        nP1, iP1 = tex.linebreak(nodecopylist(head), {looseness=tex.looseness+1, emergencystretch=0})
    else
        -- Didn't work with emergency stretch, no point to try without
        nP1, iP1 = n, i
    end

    -- Build a copy of the paragraph with decreased looseness
    local nM1, iM1 = tex.linebreak(nodecopylist(head), {looseness=tex.looseness-1})
    
    -- Reenable underfull and overfull boxes reporting
    luatexbase.remove_from_callback("hpack_quality", "hpqfilter")
    luatexbase.remove_from_callback("vpack_quality", "vpqfilter")

    -- Set color to use in the post-linebreak callback
    if iP1.prevgraf > i.prevgraf and iM1.prevgraf < i.prevgraf then
        -- Both +1 and -1 looseness would work
        LastLineColor = color_pm1
    elseif iP1s.prevgraf > i.prevgraf and iM1.prevgraf < i.prevgraf then
        -- Both +1 and -1 looseness would work, but +1 only with emergency stretch
        LastLineColor = color_pm1s
    elseif iP1.prevgraf > i.prevgraf then
        -- Only +1 looseness would work
        LastLineColor = color_p1
    elseif iP1s.prevgraf > i.prevgraf then
        -- Only +1 looseness would work and only thanks to the emergency stretch
        LastLineColor = color_p1s
    elseif iM1.prevgraf < i.prevgraf then
        -- Only -1 looseness would work
        LastLineColor = color_m1
    else
        LastLineColor = nil
    end

    return true
end

-- Callback to colorize the last line of the paragraph when ColorLastLine is true
local post_linebreak_color_last_line = function (head, groupcode)
    if LastLineColor then
        color_last_line(head)
    end
    return true
end

-- Register callbacks
luatexbase.add_to_callback("pre_linebreak_filter", pre_linebreak_test_looseness, "pre_linebreak_test_looseness")
luatexbase.add_to_callback("post_linebreak_filter", post_linebreak_color_last_line, "post_linebreak_color_last_line")
