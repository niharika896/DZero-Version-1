import { spawn } from "child_process";

class CppEngine {
    constructor() {

        this.process = spawn("../src/chess");
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

                let parsed = null;
                try {
                    parsed = JSON.parse(line);
                } catch {
                    continue; // Ignore non-JSON lines silently
                }

                if (parsed && this.callbacks.length > 0) {
                    const cb = this.callbacks.shift();
                    cb(parsed);
                }
            }
        });

        // -----------------------------------------
        // MINIMAL ERROR HANDLING
        // -----------------------------------------
        this.process.stderr.on("data", (data) => {
            console.error("[ENGINE STDERR]", data.toString());
        });

        this.process.on("close", (code) => {
            console.error("C++ engine exited with code", code);
        });

        this.process.on("error", (err) => {
            console.error("C++ engine failed:", err);
        });
    }

    // -----------------------------------------
    // SEND BOT REQUEST (NO TIMEOUT)
    // -----------------------------------------
    sendBotRequest(fen, from = "NONE", to = "NONE") {
        return new Promise((resolve) => {
            this.callbacks.push(resolve);

            this.process.stdin.write(fen + "\n");
            this.process.stdin.write(from + " " + to + "\n");
        });
    }
}

const engine = new CppEngine();
export default engine;
