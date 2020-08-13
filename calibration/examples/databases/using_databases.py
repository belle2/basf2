from caf.database import SQLiteDB
import pathlib

# Moving over to pathlib objects has nice benefits like being able to avoid a lot of os.path.join/exists calls
database_file = pathlib.Path("example.db")

if database_file.exists():
    print("Removing old database file")
    # Slightly odd api for removing a file. Use rmdir() for directories
    database_file.unlink()

# The SQLiteDB is made as a context manager so it will open a connection and close it for you
# If the database doesn't exist you ned to pass in a 'schema' that describes the tables to create
# If the database already exists, no table creation is done so you don't need to pass in a schema
with SQLiteDB(database_file, schema={"users": ["name text primary key", "age int", "gender text", "email text"]}) as db:
    # Database file and tables are created and committed automatically when opening
    # Can now insert some rows
    db.query("INSERT INTO users VALUES (?,?,?,?)", ("Tom", 25, "M", "tom@fakedomain.edu.au"))
    db.query("INSERT INTO users VALUES (?,?,?,?)", ("Charlotte", 37, "F", "charlotte@fakedomain.edu.au"))
    db.query("INSERT INTO users VALUES (?,?,?,?)", ("Taya", 36, "F", "taya@fakedomain.edu.au"))
    db.query("INSERT INTO users VALUES (?,?,?,?)", ("Simon", 36, "O", "simon@fakedomain.edu.au"))
    # Need to commit the changes using the connection object
    db.conn.commit()
    # Can now query for the values. It returns a cursor object which can be used to get the values
    cursor1 = db.query("SELECT name, email FROM users WHERE gender=?", ("F"))
    cursor2 = db.query("SELECT name, email FROM users WHERE gender=?", ("M"))

    print("Returned rows:")

    # You could iterate over the cursor to return each row
    def iterate_cursor():
        for row in cursor1:
            print(" ", row)

    # Or you could just get the first entry remaining in the statement (can be called multiple times)
    def fetch_singles():
        print(" ", cursor1.fetchone())
        print(" ", cursor1.fetchone())
        print(" ", cursor1.fetchone())  # Returns None as there are no matching rows left

    # Or you could get all rows at once, this is not advised when you expect thousands of rows. But for a smaller number its fine
    def fetch_all():
        for row in cursor1.fetchall():
            print(" ", row)

    iterate_cursor()

    # We are deliberately not using cursor2 at all. It should therefore be closed because otherwise it is an unfinished statement
    # cursor1 doesn't need to be closed because its results were fetched, even if only one of them
    cursor2.close()

print("Database connection is now closed. It will have automatically committed any remaining changes.")
print("Removing database file.")
database_file.unlink()
