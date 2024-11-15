import requests, json, hashlib, datetime, time
from mysql import connector
from dateutil.parser import parse 

pretty = False


# https://www.legis.iowa.gov/docs/code/719.1.pdf
# https://www.info.iastate.edu/

def parse_height(height: str):
    return int(height[0]) * 12 + int(height[3:5])

def exec_sql(sql: str):
    print(sql)

def main():
    print(f"Running sniffy {datetime.datetime.now()}")
    ctx = connector.connect(user="sniffy", database="inmates")
    curs = ctx.cursor()
    
    current_list = {}
    
    curs.execute("SELECT current_inmates.PID, current_inmates.AID, arrests.Bond FROM current_inmates INNER JOIN arrests ON current_inmates.AID = arrests.ID;")

    for inmate in curs:
        current_list[inmate[0]] = (inmate[1], inmate[2])    

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
        inmate["Sex"] = inmate["Sex"] == "MALE"
        inmate["FirstName"] = inmate["FirstName"].capitalize()
        inmate["LastName"] = inmate["LastName"].capitalize()
        inmate["MiddleName"] = inmate["MiddleName"].capitalize()
        inmate["BookingAgency"] = inmate["BookingAgency"].capitalize()
        inmate["Age"] = int(inmate["Age"])
        inmate["Height"] = parse_height(inmate["Height"])
        inmate["Weight"] = int(inmate["Weight"])

        print(f'Processing {inmate["FirstName"]} {inmate["LastName"]}')

        bond = 0

        for charge in inmate["Charges"]:
            bond += int(charge["BondAmount"])

        id_str = f"{inmate['LastName']}:{inmate['FirstName']}:{inmate['Sex']}:{inmate['MiddleName']}"
        id = hashlib.sha224(bytes(id_str, "latin1"))
        id = id.digest()

        if current_list.get(id, None) != None:
            print(f'{inmate["FirstName"]} {inmate["LastName"]} (ID: {id.hex()}) already exists in current inmates, AID: {current_list[id][0]}')
            del current_list[id]
            continue

        curs.reset()
        curs.execute("SELECT Arrests FROM people WHERE ID=%s", [id])

        row = curs.fetchone()

        if row == None:
            print(f'Inserting {inmate["FirstName"]} {inmate["LastName"]}')
            curs.reset()
            curs.execute("INSERT INTO people (ID,LastName,FirstName,MiddleName,Birthyear,Weight,Height,Gender,Race) VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s)", (id, inmate["LastName"], inmate["FirstName"], inmate["MiddleName"], datetime.date.today().year - inmate["Age"], inmate["Weight"], inmate["Height"], inmate["Sex"], inmate["Race"]))
        else:
            print(f'Updating {inmate["FirstName"]} {inmate["LastName"]}')
            curs.reset()
            curs.execute("UPDATE people SET Arrests=%s WHERE ID=%s", (row[0] + 1, id))

        curs.reset()

        curs.execute("INSERT INTO arrests (Date, PID, Bond) VALUES (%s, %s, %s);", (parse(inmate["ArrestDate"]).strftime('%Y-%m-%d %H:%M:%S'), id, bond))
        arrest_id = curs.lastrowid

        print(f"Created Arrest, id: {arrest_id}")

        for charge in inmate["Charges"]:
            curs.reset()
            curs.execute("SELECT ID FROM statutes WHERE ID=%s", [charge["Name"]])

            if curs.fetchone() == None:
                print(f"Creating new Statute '{charge['Name']}")
                curs.reset()
                curs.execute("INSERT INTO statutes (ID, Name, Description, Notes) VALUES (%s, %s, %s, %s)", (charge["Name"], charge["Description"], "g", "g"))
                curs.reset()
            print("Creating new charge")
            curs.execute("INSERT INTO charges (PID, AID, SID, ChargedAt, Bond, InitialBond, Agency) VALUES (%s, %s, %s, %s, %s, %s, %s)", (id, arrest_id, charge["Name"], parse(charge["Date"]).strftime('%Y-%m-%d %H:%M:%S'), charge["BondAmount"], charge["BondAmount"], inmate["BookingAgency"]))

        curs.reset()
        curs.execute("INSERT INTO current_inmates (PID, AID) VALUES (%s, %s)", (id, arrest_id))

    for key in current_list:
        print(f'Removing inmate {key.hex()}')
        curs.reset()
        curs.execute("DELETE FROM current_inmates WHERE PID=%s", [key])

    curs.close()
    ctx.commit()
    ctx.close()

if __name__ == "__main__":
    main()
