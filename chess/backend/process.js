import { spawn } from "child_process";

class CppEngine {
    constructor() {

        this.process = spawn("../src/chess.exe");
        this.callbacks = [];
        this.buffer = "";

        // -----------------------------------------
        // READ STDOUT (JSON per line)
        // -----------------------------------------
        this.process.stdout.on("data", (data) => {
            this.buffer += data.toString();

            const lines = this.buffer.split("\n");
            this.buffer = lines.pop();

            for (let raw of lines) {
                const line = raw.trim();
                if (!line) continue;

                console.log("CPP OUT:", line);

                let parsed = null;

                // Try parsing JSON
                try {
                    parsed = JSON.parse(line);
                } catch (e) {
                    console.error("NOT JSON:", line);
                    continue;
                }

                if (!parsed.move || !parsed.fen) {
                    console.error("Invalid engine JSON:", parsed);
                    continue;
                }

                if (this.callbacks.length > 0) {
                    const cb = this.callbacks.shift();
                    cb(parsed);
                }
            }
        });

        // -----------------------------------------
        // HANDLE ERRORS
        // -----------------------------------------
        this.process.stderr.on("data", (data) => {
            console.error("CPP ERROR:", data.toString());
        });

        this.process.on("close", (code) => {
            console.error("C++ engine exited with code", code);
        });

        this.process.on("error", (err) => {
            console.error("C++ engine failed:", err);
        });
    }

    // -----------------------------------------
    // SEND BOT REQUEST
    // -----------------------------------------
    sendBotRequest(fen, from = "NONE", to = "NONE") {
        return new Promise((resolve, reject) => {
            // 30-second timeout
            const timeout = setTimeout(() => {
                console.error("ENGINE TIMEOUT");
                reject(new Error("Engine timeout"));
            }, 30000);

            // Push callback
            this.callbacks.push((result) => {
                clearTimeout(timeout);
                resolve(result);
            });

            // Send data to engine
            this.process.stdin.write(fen + "\n");
            this.process.stdin.write(from + " " + to + "\n");

        });
    }
}

// Export singleton instance
const engine = new CppEngine();
export default engine;