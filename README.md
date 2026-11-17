# Real-Time Leaderboard and Ranking System

A full-stack college PBL project demonstrating practical application of advanced data structures in a real-time leaderboard management system.

## 📋 Project Overview

**Subject:** Advanced Data Structures (ADS)  
**Primary Language:** C (Backend), React.js (Frontend)  
**Database:** SQLite  
**Architecture:** REST API with real-time polling

## 🎯 Objectives

- Implement and genuinely use 6 advanced data structures in C
- Build a fully functional real-time leaderboard system
- Demonstrate practical application of data structures and algorithms
- Create a professional dark-themed dashboard with React

## 🏗️ Data Structures Implemented

| Data Structure | Purpose | Time Complexity |
|----------------|---------|-----------------|
| **AVL Tree** | Maintain players in sorted order by score | O(log n) insert/delete/search/rank |
| **Hash Table** | O(1) player lookup by ID | O(1) average insert/search/delete |
| **Max Heap** | Efficient top-K player retrieval | O(log n) insert, O(k log n) top-K |
| **Stack** | Undo functionality for score changes | O(1) push/pop |
| **Queue** | Pending score update requests | O(1) enqueue/dequeue |
| **Linked List** | Activity history log | O(1) append, O(k) recent |

## ✨ Core Features

### Player Management
- ✅ Add new players with unique names
- ✅ Delete players
- ✅ Search players by name or ID
- ✅ View player details and score history

### Score Management
- ✅ Add points to a player
- ✅ Deduct points from a player
- ✅ Set absolute score
- ✅ Undo last score operation
- ✅ Score validation

### Leaderboard
- ✅ Display all players ranked by score
- ✅ Automatic ranking updates after score changes
- ✅ Top 3 players with medal indicators (🥇🥈🥉)
- ✅ Top 10 players view
- ✅ Tie-breaking: earlier registration ranks higher
- ✅ Search and filter players

### Real-Time Updates
- ✅ Automatic polling every 5 seconds
- ✅ No manual refresh needed
- ✅ Live connection indicator

### Dashboard & Statistics
- ✅ Total registered players
- ✅ Highest score
- ✅ Average score
- ✅ Recent activity feed
- ✅ Score distribution charts
- ✅ Top players visualization
- ✅ Weekly trends
- ✅ Ranking metrics

## 🚀 Getting Started

### Prerequisites
- GCC (MinGW on Windows)
- Node.js 18+ and npm
- Git

### Build and Run

**1. Build C Backend:**
```bash
cd backend
gcc -std=c11 -Wall -Wextra -O2 -I. -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION \
    -c main.c player.c avl.c hashtable.c maxheap.c stack.c queue.c \
    linkedlist.c database.c api.c http_server.c sqlite3.c
gcc -o leaderboard.exe main.o player.o avl.o hashtable.o maxheap.o stack.o \
    queue.o linkedlist.o database.o api.o http_server.o sqlite3.o -lws2_32
```

**2. Build React Frontend:**
```bash
cd frontend
npm install
npm run build
```

**3. Copy Frontend to Backend Public Folder:**
```bash
cp -r dist/* ../backend/public/
```

**4. Run Server:**
```bash
cd ../backend
./leaderboard.exe 8080
```

**5. Open Browser:**
Navigate to `http://localhost:8080`

### Alternative: Development Mode
```bash
# Terminal 1 - Backend
cd backend && ./leaderboard.exe 8080

# Terminal 2 - Frontend
cd frontend && npm run dev
# Frontend at http://localhost:3000 (proxies API to :8080)
```

## 📡 API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/players` | List all players |
| GET | `/api/players/:id` | Get player by ID |
| POST | `/api/players` | Create new player |
| DELETE | `/api/players/:id` | Delete player |
| POST | `/api/score/add` | Add points `{id, value}` |
| POST | `/api/score/deduct` | Deduct points `{id, value}` |
| POST | `/api/score/set` | Set score `{id, value}` |
| POST | `/api/score/undo` | Undo last operation |
| GET | `/api/leaderboard?limit=10` | Get top players |
| GET | `/api/stats` | Get dashboard statistics |
| GET | `/api/activity?limit=50` | Get activity history |
| GET | `/api/search?q=name` | Search players |

## 🗄️ Database Schema

```sql
CREATE TABLE players (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    score INTEGER NOT NULL DEFAULT 0,
    created_at INTEGER NOT NULL,
    updated_at INTEGER NOT NULL
);

CREATE TABLE score_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    player_id INTEGER NOT NULL,
    old_score INTEGER NOT NULL,
    new_score INTEGER NOT NULL,
    op_type INTEGER NOT NULL,  -- 0=add, 1=deduct, 2=set, 3=create, 4=delete
    timestamp INTEGER NOT NULL,
    FOREIGN KEY (player_id) REFERENCES players(id)
);
```

## 📁 Project Structure

```
Real-Time-Leaderboard/
├── backend/
│   ├── main.c              # Entry point
│   ├── player.c/h          # Player struct
│   ├── avl.c/h             # AVL Tree
│   ├── hashtable.c/h       # Hash Table
│   ├── maxheap.c/h         # Max Heap
│   ├── stack.c/h           # Stack (Undo)
│   ├── queue.c/h           # Queue (Pending ops)
│   ├── linkedlist.c/h      # Linked List (Activity)
│   ├── database.c/h        # SQLite operations
│   ├── api.c/h             # Business logic
│   ├── http_server.c/h     # HTTP server
│   ├── sqlite3.c/h         # SQLite amalgamation
│   ├── Makefile            # Build script
│   └── public/             # Frontend build output
├── frontend/
│   ├── src/
│   │   ├── components/     # Reusable UI components
│   │   ├── pages/          # Page components
│   │   ├── hooks/          # Custom React hooks
│   │   ├── App.jsx         # Main app with routing
│   │   └── main.jsx        # Entry point
│   ├── package.json
│   └── vite.config.js
├── database/               # SQLite database files (gitignored)
├── docs/                   # Documentation
└── README.md
```

## 🧪 Testing

Run the backend and test each endpoint:

```bash
# Create players
curl -X POST http://localhost:8080/api/players -H "Content-Type: application/json" -d '{"name":"Alice"}'
curl -X POST http://localhost:8080/api/players -H "Content-Type: application/json" -d '{"name":"Bob"}'

# Add scores
curl -X POST http://localhost:8080/api/score/add -H "Content-Type: application/json" -d '{"id":1,"value":100}'
curl -X POST http://localhost:8080/api/score/add -H "Content-Type: application/json" -d '{"id":2,"value":50}'

# View leaderboard
curl http://localhost:8080/api/leaderboard

# Test undo
curl -X POST http://localhost:8080/api/score/undo

# Test delete
curl -X DELETE http://localhost:8080/api/players/2
```

## 📊 Complexity Analysis

| Operation | AVL Tree | Hash Table | Max Heap | Stack | Queue | Linked List |
|-----------|----------|------------|----------|-------|-------|-------------|
| Insert | O(log n) | O(1) | O(log n) | O(1) | O(1) | O(1) |
| Delete | O(log n) | O(1) | O(log n) | O(1) | O(1) | O(1)* |
| Search | O(log n) | O(1) | O(n) | O(n) | O(n) | O(n) |
| Rank Query | O(log n)** | - | - | - | - | - |
| Top-K | O(k) | - | O(k log n) | - | - | - |

*Delete from tail is O(1) with tail pointer  
**Requires augmented AVL with subtree sizes

## 🔧 Configuration

### Port
```bash
./leaderboard.exe 8080  # Default: 8080
```

### Database Path
```bash
./leaderboard.exe 8080 /path/to/database.db
```

## 📈 Future Enhancements

- [ ] WebSocket support for true real-time updates
- [ ] User authentication and authorization
- [ ] Tournament/match scheduling
- [ ] Export leaderboard to CSV/PDF
- [ ] Mobile-responsive improvements
- [ ] Unit tests for all data structures
- [ ] Docker containerization
- [ ] Multi-language support

## 📝 License

College PBL Project - Educational Use

## 👨‍💻 Author

Built for Advanced Data Structures (ADS) course project.