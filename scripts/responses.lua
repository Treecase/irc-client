-- Extract the username from a source string.
--  eg. for "foo!bar@baz" returns "foo"
local function extract_user(str)
    return string.match(str, "(.-)!.*") or str
end



-- TEMP: using this as an easy-to-trigger thing for tests.
function IRC.pong(b, msg)
end


function IRC.privmsg(b, msg)
    local user = extract_user(msg:prefix())
    local channel_name = msg:params(1)
    local message = msg:params(2)
    local channel = b:channels(channel_name)
    if channel ~= nil then
        channel:write(user..": "..message)
    else
        for _,channel in ipairs(b:channels()) do
            channel:write(user.." whispers: "..message)
        end
    end
end
IRC.notice = IRC.privmsg


function IRC.ping(b, msg)
    local m = Message.new("PONG :"..msg:params(1))
    b:respond(m)
end


function IRC.quit(b, msg)
    local user = extract_user(msg:prefix())
    local reason = msg:params(1)
    for _,channel in ipairs(b:channels()) do
        channel:write("<<< "..user.." quit: "..reason)
        channel:remove_user(user)
    end
end


function IRC.part(b, msg)
    local user = extract_user(msg:prefix())
    local channel_name = msg:params(1)
    local channel = b:channels(channel_name)
    if channel ~= nil then
        channel:write("<<< "..user.." left "..channel_name)
        channel:remove_user(user)
    else
        error("missing channel '"..channel_name.."'")
    end
end


function IRC.join(b, msg)
    local user = extract_user(msg:prefix())
    local channel_name = msg:params(1)
    local channel = b:channels(channel_name)
    if channel ~= nil then
        channel:add_user(user)
        channel:write(">>> "..user.." joined "..channel_name)
    else
        local channel = b:add_channel(channel_name)
        channel:add_user(user)
        b:active_channel(channel_name)
    end
end


IRC["353"] = function(b, msg)
    local channel_name = msg:params(3)
    local names = msg:params(4)

    local channel = b:channels(channel_name)
    if channel ~= nil then
        for user in string.gmatch(names, "%g+") do
            channel:add_user(user)
        end
    else
        error("missing channel '"..channel_name.."'")
    end
end
