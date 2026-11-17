import { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useApi } from '../hooks/useApi'
import './AddPlayer.css'

export default function AddPlayer() {
  const navigate = useNavigate()
  const { api, refreshAll } = useApi()
  const [name, setName] = useState('')
  const [error, setError] = useState('')
  const [submitting, setSubmitting] = useState(false)

  const handleSubmit = async (e) => {
    e.preventDefault()
    setError('')
    if (!name.trim()) {
      setError('Player name is required')
      return
    }
    setSubmitting(true)
    try {
      const player = await api.players.create(name.trim())
      refreshAll()
      navigate(`/players/${player.id}`)
    } catch (e) {
      setError(e.message)
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <div className="add-player-page">
      <div className="page-header">
        <h1>Add New Player</h1>
        <p className="subtitle">Register a new player to the leaderboard</p>
      </div>

      <div className="form-card">
        <form onSubmit={handleSubmit} className="player-form">
          <div className="form-group">
            <label htmlFor="name">Player Name</label>
            <input
              id="name"
              type="text"
              value={name}
              onChange={(e) => setName(e.target.value)}
              placeholder="Enter player name"
              maxLength={128}
              disabled={submitting}
              autoFocus
            />
            <span className="char-count">{name.length}/128</span>
          </div>

          {error && <div className="error-message">⚠ {error}</div>}

          <div className="form-actions">
            <button type="button" className="btn-secondary" onClick={() => navigate(-1)}>
              Cancel
            </button>
            <button type="submit" className="btn-primary" disabled={submitting || !name.trim()}>
              {submitting ? 'Creating...' : 'Create Player'}
            </button>
          </div>
        </form>

        <div className="form-info">
          <h4>Guidelines</h4>
          <ul>
            <li>Player names must be unique</li>
            <li>Maximum 128 characters</li>
            <li>New players start with a score of 0</li>
            <li>Rankings update automatically when scores change</li>
          </ul>
        </div>
      </div>
    </div>
  )
}