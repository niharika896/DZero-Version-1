//server.js
import express from "express";
import cors from "cors";
import botResponse from "./bot-response.js";
import dotenv from "dotenv";
const app = express();
app.use(cors());
app.use(express.json());
dotenv.config();
app.use("/botresponse", botResponse);

const PORT = process.env.PORT|| 3000;
app.listen(PORT, () => console.log("Server running on", PORT));
