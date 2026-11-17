import { useState, useEffect } from 'react'
import { useApi } from '../hooks/useApi'
import { Link } from 'react-router-dom'
import './Leaderboard.css'

export default function Leaderboard() {
  const { leaderboard, isLoading, api, refreshAll } = useApi()
  const [searchQuery, setSearchQuery] = useState('')
  const [filteredPlayers, setFilteredPlayers] = useState([])
  const [sortBy, setSortBy] = useState('rank')
  const [sortDir, setSortDir] = useState('asc')

  useEffect(() => {
    setFilteredPlayers(leaderboard)
  }, [leaderboard, searchQuery])

  useEffect(() => {
    let result = [...filteredPlayers]
    if (searchQuery) {
      const q = searchQuery.toLowerCase()
      result = result.filter(p =>
        p.name.toLowerCase().includes(q) ||
        p.id.toString().includes(q)
      )
    }
    result.sort((a, b) => {
      let aVal = a[sortBy], bVal = b[sortBy]
      if (sortBy === 'name') {
        return sortDir === 'asc' ? aVal.localeCompare(bVal) : bVal.localeCompare(aVal)
      }
      return sortDir === 'asc' ? aVal - bVal : bVal - aVal
    })
    // Reassign ranks after sorting
    result.forEach((p, i) => p.displayRank = i + 1)
    setFilteredPlayers(result)
  }, [searchQuery, sortBy, sortDir, filteredPlayers])

  return (
    <div className="leaderboard-page">
      <div className="page-header">
        <div>
          <h1>Leaderboard</h1>
          <p className="subtitle">All players ranked by score</p>
        </div>
      </div>

      <div className="leaderboard-controls">
        <div className="search-box">
          <input
            type="text"
            placeholder="Search by name or ID..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
          <span className="search-icon">🔍</span>
        </div>
        <div className="sort-controls">
          <select value={sortBy} onChange={(e) => setSortBy(e.target.value)}>
            <option value="displayRank">Rank</option>
            <option value="name">Name</option>
            <option value="score">Score</option>
            <option value="updated_at">Last Updated</option>
          </select>
          <button onClick={() => setSortDir(d => d === 'asc' ? 'desc' : 'asc')}>
            {sortDir === 'asc' ? '↑' : '↓'}
          </button>
        </div>
      </div>

      <div className="leaderboard-table-wrapper">
        {isLoading ? (
          <div className="loading">Loading leaderboard...</div>
        ) : (
          <table className="leaderboard-table">
            <thead>
              <tr>
                <th>Rank</th>
                <th>Player</th>
                <th>Score</th>
                <th>Created</th>
                <th>Last Updated</th>
                <th>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filteredPlayers.map((player) => (
                <tr key={player.id} className={player.displayRank <= 3 ? 'top-three' : ''}>
                  <td className="rank">
                    <span className={`rank-badge ${getRankClass(player.displayRank)}`}>
                      {getRankIcon(player.displayRank)} {player.displayRank}
                    </span>
                  </td>
                  <td>
                    <Link to={`/players/${player.id}`} className="player-link">
                      <div className="player-cell">
                        <span className="player-name">{player.name}</span>
                        <span className="player-id">#{player.id}</span>
                      </div>
                    </Link>
                  </td>
                  <td className="score">{player.score.toLocaleString()}</td>
                  <td className="date">{formatDate(player.created_at)}</td>
                  <td className="date">{formatDate(player.updated_at)}</td>
                  <td>
                    <Link to={`/players/${player.id}`} className="btn-icon" title="View Details">
                      👁
                    </Link>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>

      {filteredPlayers.length === 0 && !isLoading && (
        <div className="empty-state">
          <div className="empty-icon">🏆</div>
          <h3>No players found</h3>
          <p>{searchQuery ? 'Try a different search term' : 'Add your first player to get started'}</p>
          {searchQuery && <button onClick={() => setSearchQuery('')} className="btn-secondary">Clear Search</button>}
        </div>
      )}
    </div>
  )
}

function getRankClass(rank) {
  if (rank === 1) return 'gold'
  if (rank === 2) return 'silver'
  if (rank === 3) return 'bronze'
  return ''
}

function getRankIcon(rank) {
  if (rank === 1) return '🥇'
  if (rank === 2) return '🥈'
  if (rank === 3) return '🥉'
  return ''
}

function formatDate(timestamp) {
  return new Date(timestamp * 1000).toLocaleString()
}