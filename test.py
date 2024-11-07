import mysql.connector

ctx = mysql.connector.connect(user='sniffy', database='inmates')
curs = ctx.cursor()

curs.execute("SELECT * FROM arrests")

for i in curs:
    print(i)

curs.reset()

curs.execute("SELECT * FROM arrests")

for i in curs:
    print(i)

curs.close()
ctx.close()
