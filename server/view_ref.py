from flask import Flask, jsonify, render_template
import sqlite3

app = Flask(__name__)

def get_db_connection():
    conn = sqlite3.connect("water_moisture.db")
    conn.row_factory = sqlite3.Row # Allows dict-like access to rows
    return conn

@app.route("/")
def home():
    # The frontend will fetch data via JS, so we just render the template
    return render_template("index.html")

@app.route("/data/latest")
def get_data():
    conn = get_db_connection()
    cursor = conn.cursor()
    
    latest_data = {}
    for topic in ["moisture", "temp", "humidity"]:
        cursor.execute("""
            SELECT time, value
            FROM sensor_data
            WHERE topic = ?
            ORDER BY id DESC
            LIMIT 1
        """, (topic,))
        row = cursor.fetchone()
        if row:
            latest_data[topic] = {"time": row["time"], "value": row["value"]}
        else:
            latest_data[topic] = {"time": "N/A", "value": 0}
            
    conn.close()
    return jsonify(latest_data)

@app.route("/data/graph")
def get_graph():
    conn = get_db_connection()
    cursor = conn.cursor()
    
    graph_data = {"moisture": [], "temp": [], "humidity": []}
    
    for topic in ["moisture", "temp", "humidity"]:
        cursor.execute("""
            SELECT time, value
            FROM sensor_data
            WHERE topic = ?
            ORDER BY id DESC
            LIMIT 48
        """, (topic,))
        rows = cursor.fetchall()
        
        # Reverse the list so the graph plots from oldest (left) to newest (right)
        for row in reversed(rows):
            graph_data[topic].append({
                "time": row["time"],
                "value": row["value"]
            })
            
    conn.close()
    return jsonify(graph_data)


@app.route("/data/last-watered")
def get_last_watered():
    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute("""
        SELECT time
        FROM (
            SELECT
                time,
                value,
                LAG(value) OVER (ORDER BY id) AS previous_value
            FROM sensor_data
            WHERE topic = 'moisture'
        )
        WHERE value >= 2000
          AND previous_value < 2000
        ORDER BY time DESC
        LIMIT 1
    """)

    row = cursor.fetchone()

    if row:
        last_watered = row["time"]
    else:
        last_watered = "N/A"

    conn.close()

    return jsonify({"time": last_watered})


if __name__ == "__main__":
    app.run(debug=True)