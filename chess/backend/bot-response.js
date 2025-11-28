import express from "express";
import engine from "./process.js";

const router = express.Router();

router.post("/", async (req, res) => {
    try {
        const { fen, from, to } = req.body;

        if (!fen) {
            return res.status(400).json({ success: false, error: "FEN is required" });
        }

        const output = await engine.sendBotRequest(fen, from, to);

        res.json({ 
            success: true, 
            botMove: output.move,  // e.g., "E7 E5"
            fen: output.fen        // Updated FEN after bot move
        });

    } catch (err) {
        console.error("Bot response error:", err);
        res.status(500).json({ success: false, error: err.message });
    }
});

export default router;