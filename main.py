import requests, json, hashlib
from dateutil import tz

pretty = False


# https://www.legis.iowa.gov/docs/code/719.1.pdf
# https://www.info.iastate.edu/

def exec_sql(sql: str):
    print(sql)

def main():
    new_people = []
    updated_people = []
    new_arrests = []
    new_mugshots = []
    new_charges = []
    
    exec_sql("SELECT current_inmates.PID, current_inmates.AID, arrests.Bond FROM current_inmates INNER JOIN arrests ON current_inmates.AID = arrests.AID;")
    res = requests.get("https://centraliowa.policetocitizen.com/Inmates/Catalog")
    cookies = res.headers.get("Set-Cookie")

    cookies = str(cookies)

    cookies = cookies.split(",")
    i = 0
    cookie_dict = {}

    header_dict = {
        "Sec-Fetch-Mode": "cors",
        "Sec-Fetch-Dest": "empty",
        "Referrer": "https://centraliowa.policetocitizen.com/Inmates/Catalog",
        "Priority": "u=1, i",
        "Content-Type": "application/json"
    }

    # I hate this but it works, rewrite this in C eventually
    while i < len(cookies):
        cookies[i] = cookies[i].strip()
        if cookies[i].count("expires=") and cookies[i + 1] and cookies[i + 1][1:3].isdigit():
            cookies[i] = ' '.join(cookies[i:i + 2])
            cookies.pop(i + 1)

        temp = cookies[i].split(";")[0].split("=")
        if len(temp) == 2:
            cookie_dict[temp[0]] = temp[1]
        i += 1

    header_dict["X-Xsrf-Token"] = cookie_dict["XSRF-TOKEN"]

    opts = {
        "FilterOptionsParameters": {
            "IntersectionSearch": True,
            "SearchText": "",
            "Parameters": []
        },
        "IncludeCount": True,
        "PagingOptions": {
            "SortOptions": [
            {
                "Name": "ArrestDate",
                "SortDirection": "Descending",
                "Sequence": 1
            }
            ],
            "Take": 100,
            "Skip": 0
        }
    }

    roster_json = requests.post("https://centraliowa.policetocitizen.com/api/Inmates/241", json=opts, cookies=cookie_dict, headers=header_dict).json()

    for inmate in roster_json["Inmates"]:
        print(json.dumps(inmate), sep="\n")
        id_str = f"{inmate["LastName"]}:{inmate["FirstName"]}:{inmate["Sex"]}:{inmate["MiddleName"]}"
        id = hashlib.sha224(bytes(id_str, "latin1"))
        id = id.digest()

        print(id)

        break
        # if pid is in current_inmates just check to see if bond needs to be adjusted
        if False:
            print("TODO")
        else:
            # Check and see if the PID exists in people already, if not create a new person
            exec_sql("SELECT ID, Arrests FROM people WHERE ID = id;")
            
            # if they dont exist
            if True:
                new_people.append(inmate)
            else:
                updated_people.append(inmate)


    exec_sql("INSERT INTO people (ID, LastName, FirstName, MiddleName, Birthyear, Weight, Height, Gender, Race)")

if __name__ == "__main__":
    main()
