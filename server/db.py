import sqlite3

connection = sqlite3.connect("water_moisture.db")
cursor = connection.cursor()

# Creating a more flexible table to hold data for any topic
cursor.execute("""
CREATE TABLE IF NOT EXISTS sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,  
    time TEXT NOT NULL,
    topic TEXT NOT NULL,
    value REAL NOT NULL
)
""")

connection.commit()
connection.close()
print("Database setup complete.")