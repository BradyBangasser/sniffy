META_DATA = {
    facility_name = "Anoka County Jail",
    facility_address = "325 East Jackson Street, Anoka, MN 55303",
    facility_cap = 238,
    start_time = 2,
    run_interval = 30,
    state_code = "MN",
    cache = true,
    inaccurate_time = true,
}

local request = require("http.request")
local json = require("dkjson")


function FETCH()
    local LIMIT = 0

    -- https://centraliowa.policetocitizen.com/Inmates/Catalog
    local req = request.new_from_uri("https://incustodysearch.co.anoka.mn.us/JailInfoForPublic/inmates_json.aspx")
    local _, stream = req:go()

    local obj = json.decode(stream:get_body_as_string())
    obj = obj["inmates"]

    local booking_ids = {}
    local n = 0

    for _,v in pairs(obj) do
        if n > 1000 then
            break
        end
        local booking_info = {}
        booking_info.released = v[6] ~= ""
        booking_info.first_name = v[2]
        booking_info.middle_name = v[3]
        booking_info.last_name = v[1]
        booking_info.arrest_date = v[7]
        booking_info.b_id = v[4]
        booking_ids[v[4]] = json.encode(booking_info)
        n = n + 1
    end

    return 0, json.encode(booking_ids)
end

function FETCH_INCREMENTAL(id)
    local request_uri = "https://incustodysearch.co.anoka.mn.us/JailInfoForPublic/inmate_json.aspx?Booking_Number=" .. id
    local req = request.new_from_uri(request_uri)
    local _, stream = req:go()

    local stream_str = stream:get_body_as_string()

    local person = json.decode(stream_str)

    if person == nil then
        -- print("FAILED TO PARSE: " .. stream_str)

        local debug_data = {
            id,
            result = stream_str,
            request_uri
        }

        return 1, json.encode(debug_data)
    end

    local arrest = person["Booking"][1]

    arrest["charges"] = {}

    for j,v in pairs(person["Charges"]) do
        arrest["charges"][j] = {}
        arrest["charges"][j].notes = "TIME NOT ACCURATE"
        arrest["charges"][j]["charged_at"] = arrest["ArrestDate"]
        local description = v["ChargeDescription"]
        local i = string.find(description, "-[^-]*$")

        if i == nil then
            if description == "Hold Civil Commitment" then
                arrest["charges"][j]["description"] = description
                arrest["charges"][j]["statute"] = description
            else
                arrest["charges"][j]["description"] = description
                arrest["charges"][j]["statute"] = "unknown"
            end
            goto continue
        end

        i = i - 1

        local charge_string = string.sub(description, 0, i)
        arrest["charges"][j]["description"] = charge_string

        local m = string.find(description, "{", i + 2)

        if m == nil then
            if charge_string == "PAROLE VIOLATION" then
                arrest["charges"][j]["statute"] = charge_string
            else
                arrest["charges"][j]["statute"] = "unknown"
            end

            goto continue
        end

        local statute = string.sub(description, m + 1, -2)
        arrest["charges"][j]["statute"] = statute
        ::continue::
    end

    arrest["bond"] = 0
    arrest["dockets"] = {}

    for _, v in pairs(person["BookingBails"]) do
        arrest["bond"] = arrest["bond"] + tonumber(v["Bail_Amount"])
        table.insert(arrest["dockets"], v["Court_Number"])
    end

    return 0, json.encode(arrest)
end
