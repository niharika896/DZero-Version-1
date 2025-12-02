import { useState, useRef, useEffect } from "react";
import { Chessboard } from "react-chessboard";
import { Chess } from "chess.js";
import type { SquareHandlerArgs } from "react-chessboard";
import type { Square } from "chess.js";

export default function App() {
    const START_FEN = "rnbk2nr/6pp/3p4/1q6/4PQ2/2P2N2/PB3PPP/RN3RK1 w - - 0 1";  

const chessGameRef = useRef(new Chess());

    const chessGame = chessGameRef.current;

    const [chessPosition, setChessPosition] = useState(chessGame.fen());
    const [moveFrom, setMoveFrom] = useState('');
    const [optionSquares, setOptionSquares] = useState({});
    const [turn, setTurn] = useState<'w'|'b'>('w');
    const [gameStatus, setGameStatus] = useState<string>('');

    // Sound effects
    const moveSoundRef = useRef<HTMLAudioElement | null>(null);
    const captureSoundRef = useRef<HTMLAudioElement | null>(null);
    const castleSoundRef = useRef<HTMLAudioElement | null>(null);
    const checkSoundRef = useRef<HTMLAudioElement | null>(null);
    const gameEndSoundRef = useRef<HTMLAudioElement | null>(null);

    useEffect(() => {
        moveSoundRef.current = new Audio('https://images.chesscomfiles.com/chess-themes/sounds/_MP3_/default/move-self.mp3');
        captureSoundRef.current = new Audio('https://images.chesscomfiles.com/chess-themes/sounds/_MP3_/default/capture.mp3');
        castleSoundRef.current = new Audio('https://images.chesscomfiles.com/chess-themes/sounds/_MP3_/default/castle.mp3');
        checkSoundRef.current = new Audio('https://images.chesscomfiles.com/chess-themes/sounds/_MP3_/default/move-check.mp3');
        gameEndSoundRef.current = new Audio('https://images.chesscomfiles.com/chess-themes/sounds/_MP3_/default/game-end.mp3');
    }, []);

    function playSound(moveData: { captured?: boolean; flags?: string }) {
        // Check for castle
        if (moveData.flags?.includes('k') || moveData.flags?.includes('q')) {
            castleSoundRef.current?.play();
        }
        // Check for capture
        else if (moveData.captured) {
            captureSoundRef.current?.play();
        }
        // Regular move
        else {
            moveSoundRef.current?.play();
        }

        // Check game status
        setTimeout(() => {
            if (chessGame.isCheckmate()) {
                gameEndSoundRef.current?.play();
                const winner = chessGame.turn() === 'w' ? 'Black' : 'White';
                setGameStatus(`Checkmate! ${winner} wins!`);
                alert(`Checkmate! ${winner} wins!`);
            } else if (chessGame.isStalemate()) {
                gameEndSoundRef.current?.play();
                setGameStatus("Draw by stalemate!");
                alert("Draw by stalemate!");
            } else if (chessGame.isDraw()) {
                gameEndSoundRef.current?.play();
                setGameStatus("Draw!");
                alert("Draw!");
            } else if (chessGame.isCheck()) {
                checkSoundRef.current?.play();
                setGameStatus("Check!");
            } else {
                setGameStatus('');
            }
        }, 100);
    }

    async function makeBotMove(playerFrom: string, playerTo: string) {
    try {
        const res = await fetch('https://dzero-version-1-server.onrender.com/botresponse', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                fen: chessGame.fen(),
                from: playerFrom,
                to: playerTo
            })
        });

        if (!res.ok) throw new Error(`HTTP ${res.status}`);

        const data = await res.json();

        if (!data?.botMove) {
            console.warn("⚠️ Bot returned no move");
            setTurn('w');
            return;
        }

        // ---- NEW: JSON move parsing ----
        // botMove = "e7e5" or "e7e8q"
        const moveStr = data.botMove.trim().toLowerCase();

        const botFrom = moveStr.slice(0, 2);   // e7
        const botTo = moveStr.slice(2, 4);     // e5
        const promotion = moveStr[4] || undefined;

        const moveResult = chessGame.move({
            from: botFrom,
            to: botTo,
            promotion: promotion
        });

        if (moveResult) {
            playSound({
                captured: moveResult.captured !== undefined,
                flags: moveResult.flags
            });
        }

        // ---- NEW: Set engine FEN to ensure sync ----
        try {
    if (data.fen) {
        chessGame.load(data.fen);   // try engine FEN
    }
} catch (e) {
    console.warn("⚠️ Engine returned invalid FEN, ignoring engine FEN:", data.fen);
    // Just rely on chess.js internal board instead
}


        setChessPosition(chessGame.fen());
        setTurn('w');

    } catch (err) {
        console.error("❌ Bot move failed", err);
        setTurn('w');
    }
}

    function onSquareClick({ square, piece }: SquareHandlerArgs) {
    if (turn !== 'w') return;

    if (!moveFrom && piece) {
        const hasMoves = getMoveOptions(square as Square);
        if (hasMoves) setMoveFrom(square);
        return;
    }

    const moves = chessGame.moves({ square: moveFrom as Square, verbose: true });
    const foundMove = moves.find((m) => m.from === moveFrom && m.to === square);

    // Not a valid move → select new piece or clear selection
    if (!foundMove) {
        const canSelect = getMoveOptions(square as Square);
        setMoveFrom(canSelect ? square : '');
        return;
    }

    let moveResult;
    try {
        moveResult = chessGame.move({ from: moveFrom, to: square, promotion: 'q' });
    } catch {
        const canSelect = getMoveOptions(square as Square);
        setMoveFrom(canSelect ? square : '');
        return;
    }

    if (moveResult) {
        playSound({
            captured: moveResult.captured !== undefined,
            flags: moveResult.flags
        });
    }

    setChessPosition(chessGame.fen());
    setMoveFrom('');
    setOptionSquares({});

    setTurn('b');
    makeBotMove(moveFrom, square);
}


    function getMoveOptions(square: Square) {
        const moves = chessGame.moves({ square, verbose: true });
        if (moves.length === 0) { 
            setOptionSquares({}); 
            return false; 
        }
        
        const newSquares: Record<string, React.CSSProperties> = {};
        for (const move of moves) {
            newSquares[move.to] = {
                background: chessGame.get(move.to) && chessGame.get(move.to)?.color !== chessGame.get(square)?.color 
                    ? 'radial-gradient(circle, rgba(0,0,0,.1) 85%, transparent 85%)'
                    : 'radial-gradient(circle, rgba(0,0,0,.1) 25%, transparent 25%)',
                borderRadius: '50%'
            };
        }
        newSquares[square] = { background: 'rgba(255, 255, 0, 0.4)' };
        setOptionSquares(newSquares);
        return true;
    }

        const chessboardOptions = {
      allowDragging: false,
      onSquareClick,
      position: chessPosition,
      squareStyles: optionSquares,
      id: 'click-to-move'
    };

    return (
    <div className="max-h-screen w-[50%]">

        <Chessboard options={chessboardOptions} />

        {gameStatus && (
            <div
                className={`
                    absolute left-10 -translate-x-9 bottom-5
                    px-5 py-3 rounded-xl border backdrop-blur-md
                    text-lg font-semibold text-center animate-toast
                    shadow-lg
                    ${gameStatus.includes('Checkmate')
                        ? 'bg-red-900/70 text-yellow-200 border-red-300'
                        : gameStatus.includes('Check')
                        ? 'bg-yellow-700/70 text-black border-yellow-300'
                        : 'bg-slate-800/70 text-white border-slate-500'}
                `}
                style={{
                    minWidth: "200px"
                }}
            >
                {gameStatus}
            </div>
        )}
    </div>
);



}