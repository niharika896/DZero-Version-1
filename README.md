## Compile & Run Chess Engine

```bash
cd DZero\chess\src
g++ -std=c++20 core/*.cpp core/PreComputedTables/*.cpp -o chess.exe
./chess.exe
```
## Starting frontend
```bash
cd DZero\chess\frontend
npm i
npm run dev
```

## Starting backend
```bash
cd DZero\chess\backend
npm i
nodemon server.js
```
