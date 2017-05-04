import sys, sqlite3

def test():
    db=sqlite3.connect('./database/backup.db'); c=db.cursor();  c.execute('SELECT * FROM ph');
    N=10
    j=1
    temporal = {}
    for i in c:
        if j<=N:
            temporal[j]=[i[1],i[2]]
            j+=1

        else:
            break


    testa = []
    testb = []

    for i in range(1,len(temporal)):
        testa.append(str(temporal[i][0]))
        testb.append(str(temporal[i][1]))


    test = [testa,testb]

    return test
