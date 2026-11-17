import { useState, useEffect } from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import { useApi, usePlayer } from '../hooks/useApi'
import './PlayerDetails.css'

export default function PlayerDetails() {
  const { id } = useParams()
  const navigate = useNavigate()
  const { api, refreshAll } = useApi()
  const { player, loading, error, refetch } = usePlayer(id)
  const [deleting, setDeleting] = useState(false)
  const [showDeleteConfirm, setShowDeleteConfirm] = useState(false)

  const handleDelete = async () => {
    setDeleting(true)
    try {
      await api.players.delete(parseInt(id))
      refreshAll()
      navigate('/players')
    } catch (e) {
      alert(e.message)
    } finally {
      setDeleting(false)
    }
  }

  if (loading) {
    return (
      <div className="player-details loading-state">
        <div className="spinner"></div>
        <p>Loading player details...</p>
      </div>
    )
  }

  if (error || !player) {
    return (
      <div className="player-details error-state">
        <h2>Player Not Found</h2>
        <p>{error || 'The requested player does not exist'}</p>
        <button className="btn-primary" onClick={() => navigate('/players')}>
          Back to Players
        </button>
      </div>
    )
  }

  return (
    <div className="player-details">
      <div className="page-header">
        <div className="header-left">
          <button className="btn-back" onClick={() => navigate(-1)}>← Back</button>
          <div>
            <h1>{player.name}</h1>
            <p className="subtitle">Player Details & Score History</p>
          </div>
        </div>
        <div className="header-actions">
          <button className="btn-secondary" onClick={() => navigate(`/scores?player=${player.id}`)}>
            ⚡ Update Score
          </button>
          <button className="btn-secondary danger" onClick={() => setShowDeleteConfirm(true)}>
            🗑 Delete
          </button>
        </div>
      </div>

      <div className="details-grid">
        <section className="panel main-info">
          <div className="player-profile">
            <div className="profile-header">
              <div className="profile-avatar">
                {getInitials(player.name)}
              </div>
              <div className="profile-info">
                <h2>{player.name}</h2>
                <div className="profile-meta">
                  <span className="player-id">ID: #{player.id}</span>
                  <span className={`rank-badge ${getRankClass(player.rank)}`}>
                    {getRankIcon(player.rank)} Rank #{player.rank}
                  </span>
                </div>
              </div>
            </div>

            <div className="profile-stats">
              <div className="stat large">
                <span className="stat-label">Current Score</span>
                <span className="stat-value score">{player.score.toLocaleString()}</span>
              </div>
              <div className="stat">
                <span className="stat-label">Registered</span>
                <span className="stat-value">{formatDate(player.created_at)}</span>
              </div>
              <div className="stat">
                <span className="stat-label">Last Updated</span>
                <span className="stat-value">{formatDate(player.updated_at)}</span>
              </div>
            </div>
          </div>
        </section>

        <section className="panel score-history">
          <div className="panel-header">
            <h3>Score History</h3>
          </div>
          <div className="history-list">
            {player.scoreHistory && player.scoreHistory.length > 0 ? (
              player.scoreHistory.map((entry, idx) => (
                <div key={idx} className="history-item">
                  <div className="history-icon {getOpClass(entry.op_type)}">
                    {getOpIcon(entry.op_type)}
                  </div>
                  <div className="history-details">
                    <div className="history-main">
                      <span className="history-op">{getOpLabel(entry.op_type)}</span>
                      <span className={`history-change ${getChangeClass(entry.op_type, entry.old_score, entry.new_score)}`}>
                        {getChangeText(entry.op_type, entry.old_score, entry.new_score)}
                      </span>
                    </div>
                    <div className="history-time">{formatDate(entry.timestamp)}</div>
                  </div>
                </div>
              ))
            ) : (
              <div className="empty-history">
                <span className="icon">📊</span>
                <p>No score history yet</p>
                <span>Score changes will appear here</span>
              </div>
            )}
          </div>
        </section>
      </div>

      {showDeleteConfirm && (
        <div className="modal-overlay" onClick={() => setShowDeleteConfirm(false)}>
          <div className="modal" onClick={e => e.stopPropagation()}>
            <h3>Delete Player</h3>
            <p>Are you sure you want to delete <strong>{player.name}</strong>?</p>
            <p className="warning">This action cannot be undone. All score history will be lost.</p>
            <div className="modal-actions">
              <button className="btn-secondary" onClick={() => setShowDeleteConfirm(false)}>Cancel</button>
              <button className="btn-primary danger" onClick={handleDelete} disabled={deleting}>
                {deleting ? 'Deleting...' : 'Delete Player'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  )
}

function getInitials(name) {
  return name.split(' ').map(n => n[0]).join('').toUpperCase().slice(0, 2)
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

function getOpClass(opType) {
  switch (opType) {
    case 0: return 'add'
    case 1: return 'deduct'
    case 2: return 'set'
    case 3: return 'create'
    case 4: return 'delete'
    default: return ''
  }
}

function getOpIcon(opType) {
  switch (opType) {
    case 0: return '+'
    case 1: return '−'
    case 2: return '='
    case 3: return '+'
    case 4: return '🗑'
    default: return '•'
  }
}

function getOpLabel(opType) {
  switch (opType) {
    case 0: return 'Points Added'
    case 1: return 'Points Deducted'
    case 2: return 'Score Set'
    case 3: return 'Player Created'
    case 4: return 'Player Deleted'
    default: return 'Unknown'
  }
}

function getChangeClass(opType, oldScore, newScore) {
  if (opType === 0 || (opType === 2 && newScore > oldScore)) return 'positive'
  if (opType === 1 || (opType === 2 && newScore < oldScore)) return 'negative'
  return 'neutral'
}

function getChangeText(opType, oldScore, newScore) {
  if (opType === 3) return 'Player created with initial score 0'
  if (opType === 4) return 'Player deleted'
  if (opType === 0) return `+${newScore - oldScore} (${oldScore} → ${newScore})`
  if (opType === 1) return `-${oldScore - newScore} (${oldScore} → ${newScore})`
  if (opType === 2) return `${oldScore} → ${newScore}`
  return `${oldScore} → ${newScore}`
}