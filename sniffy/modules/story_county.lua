function FETCH ()
    local http = require('socket.http')
    local request = require("http.request")
    local header = require("http.headers")
    local cookie = require("http.cookie")

    -- https://centraliowa.policetocitizen.com/Inmates/Catalog
    local req = request.new_from_uri("https://centraliowa.policetocitizen.com/Inmates/Catalog")
    local rheaders, stream = req:go()
    local rcookies, _, p = cookie.parse_setcookie(rheaders:get("set-cookie"))

    local i = 1
    local c = rheaders:get_as_sequence("set-cookie")
    local xsrft = nil
    local cstore = {}

    while i < c["n"] do
        local co, value, opts = cookie.parse_setcookie(c[i])
        if co == "XSRF-TOKEN" then
            xsrft = value
        end

        cstore[co] = value

        i = i + 1
    end


    if xsrft == nil then
        print("X-Xsrf-Token is null")
        return ""
    end

    -- header_dict = {
    --     "Sec-Fetch-Mode": "cors",
    --     "Sec-Fetch-Dest": "empty",
    --     "Referrer": "https://centraliowa.policetocitizen.com/Inmates/Catalog",
    --     "Priority": "u=1, i",
    --     "Content-Type": "application/json"
    -- }

    req = request.new_from_uri("https://centraliowa.policetocitizen.com/api/Inmates/241")
    req.headers:append("X-Xsrf-Token", xsrft)
    req.headers:append("Sec-Fetch-Mode", "cors")
    req.headers:append("Sec-Fetch-Dest", "empty")
    req.headers:append("Referrer", "https://centraliowa.policetocitizen.com/Inmates/Catalog")
    req.headers:append("Priority", "u=1, i")
    req.headers:append("Content-Type", "application/json")
    req.headers:upsert(":method", "POST")

    for k,v in pairs(cstore) do
        req.headers:append("Cookie", k .. "=" .. v .. ";")
    end

    req:set_body('{"FilterOptionsParameters":{"IntersectionSearch":true,"SearchText":"","Parameters":[]},"IncludeCount":true,"PagingOptions":{"SortOptions":[{"Name":"ArrestDate","SortDirection":"Descending","Sequence":1}],"Take":10,"Skip":0}}')

    rheaders, stream = req:go()

    return stream:get_body_as_string()
end
