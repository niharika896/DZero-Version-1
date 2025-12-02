import express from "express";
import engine from "./process.js";

const router = express.Router();

router.post("/", async (req, res) => {
    try {
        const { fen, from = "NONE", to = "NONE" } = req.body;


        if (!fen || typeof fen !== "string") {
            return res.status(400).json({
                success: false,
                error: "Invalid or missing FEN"
            });
        }

        // -----------------------
        // CALL ENGINE
        // -----------------------
        const result = await engine.sendBotRequest(fen, from, to);

        if (!result || !result.move || !result.fen) {
            console.error("Invalid engine result:", result);
            return res.status(500).json({
                success: false,
                error: "Engine returned invalid response"
            });
        }
        // -----------------------
        // SEND SUCCESS RESPONSE
        // -----------------------
        return res.json({
            success: true,
            botMove: result.move,   // "e2e4" or "e7e8q"
            fen: result.fen
        });

    } catch (err) {
        console.error("Bot response error:", err);

        return res.status(500).json({
            success: false,
            error: err.message || "Engine error"
        });
    }
});

export default router;