function IRC.ping(msg)
    local m = Message.new("PONG :"..msg:params(1))
    local i = Interaction.respond(m)
    return i
end