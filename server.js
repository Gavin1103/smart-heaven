const express = require("express");
const cors = require("cors");
const path = require("path");
const { Pool } = require("pg");

const app = express();

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));


// DATABASE CONNECTION
const pool = new Pool({
    user: "postgres",
    host: "localhost",
    database: "smartheaven",
    password: "denzel",
    port: 5432,
});


// DEBUG: CHECK WHICH DATABASE IS USED
app.get("/check-db", async (req, res) => {

    try {

        const dbName = await pool.query("SELECT current_database()");
        const tables = await pool.query(
            "SELECT table_name FROM information_schema.tables WHERE table_schema='public'"
        );

        res.json({
            connected_database: dbName.rows[0].current_database,
            tables_found: tables.rows
        });

    } catch (error) {

        console.error(error);

        res.send("Database check failed");

    }

});


// DEBUG: CHECK DEVICE TABLE CONTENT
app.get("/overview-data", async (req, res) => {

    try {

        const result = await pool.query(
            "SELECT * FROM public.device ORDER BY device_id ASC"
        );

        console.log("DATA FOUND:", result.rows);

        res.json(result.rows);

    } catch (error) {

        console.error("DATABASE ERROR:", error);

        res.status(500).send("Database error");

    }

});


app.listen(3000, () => {
    console.log("Server running on http://localhost:3000");
});