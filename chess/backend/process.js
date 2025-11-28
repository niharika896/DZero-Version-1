import { spawn } from "child_process";
import path from "path";
class CppEngine {
    constructor() {
        this.process = spawn(path.join(__dirname, "../src/chess"));
        this.callbacks = [];
        this.buffer = "";
        this.pendingLines = [];

        this.process.stdout.on("data", (data) => {
            this.buffer += data.toString();
            
            let lines = this.buffer.split("\n");
            this.buffer = lines.pop() || "";
            
            for (const line of lines) {
                const text = line.trim();
                if (text) {
                    console.log("CPP OUT:", text);
                    this.pendingLines.push(text);
                    
                    // We expect 2 lines: MOVE then FEN
                    if (this.pendingLines.length >= 2 && this.callbacks.length > 0) {
                        const cb = this.callbacks.shift();
                        const response = {
                            move: this.pendingLines[0],
                            fen: this.pendingLines[1]
                        };
                        this.pendingLines = [];
                        cb(response);
                    }
                }
            }
        });

        this.process.stderr.on("data", (data) => {
            console.error("CPP ERROR:", data.toString());
        });

        this.process.on("close", (code) => {
            console.error("CPP process exited with code", code);
        });

        this.process.on("error", (err) => {
            console.error("CPP process error:", err);
        });
    }

    sendBotRequest(fen, from = "NONE", to = "NONE") {
        return new Promise((resolve, reject) => {
            const timeout = setTimeout(() => {
                console.error("Timeout! Pending lines:", this.pendingLines);
                this.pendingLines = [];
                reject(new Error("C++ engine timeout"));
            }, 30000);  // 30 seconds

            this.callbacks.push((result) => {
                clearTimeout(timeout);
                resolve(result);
            });

            console.log("Sending to C++:", fen);
            console.log("Sending to C++:", from, to);
            
            this.process.stdin.write(fen + "\n");
            this.process.stdin.write(from + " " + to + "\n");
        });
    }
}

const engine = new CppEngine();
export default engine;
