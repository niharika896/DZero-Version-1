//server.js
import express from "express";
import cors from "cors";
import botResponse from "./bot-response.js";

const app = express();
app.use(cors());
app.use(express.json());

app.use("/botresponse", botResponse);

const PORT = 3000;
app.listen(PORT, () => console.log("Server running on", PORT));
