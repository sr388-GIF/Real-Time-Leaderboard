import { useState } from 'react'
import { Link } from 'react-router-dom'
import { useApi } from '../hooks/useApi'
import './PlayerManagement.css'

export default function PlayerManagement() {
  const { players, isLoading, api, refreshAll } = useApi()
  const [deletingId, setDeletingId] = useState(null)

  const handleDelete = async (id, name) => {
    if (!window.confirm(`Delete player "${name}"? This cannot be undone.`)) return
    setDeletingId(id)
    try {
      await api.players.delete(id)
      refreshAll()
    } catch (e) {
      alert(e.message)
    } finally {
      setDeletingId(null)
    }
  }

  return (
    <div className="player-management">
      <div className="page-header">
        <div>
          <h1>Player Management</h1>
          <p className="subtitle">Manage all registered players</p>
        </div>
        <Link to="/players/add" className="btn-primary">
          <span>➕</span> Add Player
        </Link>
      </div>

      <div className="table-wrapper">
        {isLoading ? (
          <div className="loading">Loading players...</div>
        ) : (
          <table className="players-table">
            <thead>
              <tr>
                <th>ID</th>
                <th>Name</th>
                <th>Score</th>
                <th>Rank</th>
                <th>Created</th>
                <th>Last Updated</th>
                <th>Actions</th>
              </tr>
            </thead>
            <tbody>
              {players.map((player, idx) => (
                <tr key={player.id}>
                  <td className="id-cell">#{player.id}</td>
                  <td>
                    <Link to={`/players/${player.id}`} className="player-name-link">
                      {player.name}
                    </Link>
                  </td>
                  <td className="score">{player.score.toLocaleString()}</td>
                  <td>
                    <span className={`rank-badge ${getRankClass(idx + 1)}`}>
                      {getRankIcon(idx + 1)} {idx + 1}
                    </span>
                  </td>
                  <td className="date">{formatDate(player.created_at)}</td>
                  <td className="date">{formatDate(player.updated_at)}</td>
                  <td>
                    <div className="action-buttons">
                      <Link to={`/players/${player.id}`} className="btn-icon" title="View">
                        👁
                      </Link>
                      <Link to={`/scores?player=${player.id}`} className="btn-icon" title="Update Score">
                        ⚡
                      </Link>
                      <button
                        className="btn-icon danger"
                        onClick={() => handleDelete(player.id, player.name)}
                        disabled={deletingId === player.id}
                        title="Delete"
                      >
                        {deletingId === player.id ? '⏳' : '🗑'}
                      </button>
                    </div>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>

      {players.length === 0 && !isLoading && (
        <div className="empty-state">
          <div className="empty-icon">👥</div>
          <h3>No players yet</h3>
          <p>Add your first player to get started</p>
          <Link to="/players/add" className="btn-primary">Add Player</Link>
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