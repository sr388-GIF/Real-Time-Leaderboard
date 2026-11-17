import { useState, useEffect } from 'react'
import { useSearchParams } from 'react-router-dom'
import { useApi } from '../hooks/useApi'
import './UpdateScores.css'

export default function UpdateScores() {
  const [searchParams] = useSearchParams()
  const preselectedId = searchParams.get('player')
  const { players, api, refreshAll } = useApi()

  const [selectedPlayerId, setSelectedPlayerId] = useState(preselectedId ? parseInt(preselectedId) : '')
  const [operation, setOperation] = useState('add')
  const [value, setValue] = useState('')
  const [error, setError] = useState('')
  const [success, setSuccess] = useState('')
  const [submitting, setSubmitting] = useState(false)
  const [selectedPlayer, setSelectedPlayer] = useState(null)

  useEffect(() => {
    if (selectedPlayerId) {
      const p = players.find(pl => pl.id === parseInt(selectedPlayerId))
      setSelectedPlayer(p || null)
    } else {
      setSelectedPlayer(null)
    }
  }, [selectedPlayerId, players])

  const handleSubmit = async (e) => {
    e.preventDefault()
    setError('')
    setSuccess('')

    const id = parseInt(selectedPlayerId)
    const val = parseInt(value)

    if (!id || isNaN(val) || val <= 0) {
      setError('Please select a player and enter a valid positive number')
      return
    }

    setSubmitting(true)
    try {
      let result
      if (operation === 'add') result = await api.scores.add(id, val)
      else if (operation === 'deduct') result = await api.scores.deduct(id, val)
      else if (operation === 'set') result = await api.scores.set(id, val)

      setSuccess(`Success! ${result.name} now has ${result.score.toLocaleString()} points (Rank #${result.rank})`)
      setValue('')
      refreshAll()
    } catch (e) {
      setError(e.message)
    } finally {
      setSubmitting(false)
    }
  }

  const handleUndo = async () => {
    setError('')
    setSuccess('')
    setSubmitting(true)
    try {
      const result = await api.scores.undo()
      setSuccess(`Undo successful! ${result.name} reverted to ${result.score.toLocaleString()} points`)
      setValue('')
      refreshAll()
    } catch (e) {
      setError(e.message)
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <div className="update-scores-page">
      <div className="page-header">
        <h1>Update Scores</h1>
        <p className="subtitle">Modify player scores and manage rankings</p>
      </div>

      <div className="scores-grid">
        <div className="form-card">
          <h3>Score Operation</h3>
          <form onSubmit={handleSubmit} className="score-form">
            <div className="form-group">
              <label>Player</label>
              <select
                value={selectedPlayerId}
                onChange={(e) => setSelectedPlayerId(e.target.value)}
                disabled={submitting}
                required
              >
                <option value="">Select a player...</option>
                {players.map(p => (
                  <option key={p.id} value={p.id}>
                    {p.name} (Score: {p.score.toLocaleString()}, Rank: #{players.indexOf(p) + 1})
                  </option>
                ))}
              </select>
            </div>

            <div className="form-group">
              <label>Operation</label>
              <div className="operation-tabs">
                {[
                  { value: 'add', label: 'Add Points', desc: 'Increase score' },
                  { value: 'deduct', label: 'Deduct Points', desc: 'Decrease score' },
                  { value: 'set', label: 'Set Score', desc: 'Set exact score' }
                ].map(op => (
                  <button
                    key={op.value}
                    type="button"
                    className={`op-tab ${operation === op.value ? 'active' : ''}`}
                    onClick={() => setOperation(op.value)}
                    disabled={submitting}
                  >
                    <span className="op-label">{op.label}</span>
                    <span className="op-desc">{op.desc}</span>
                  </button>
                ))}
              </div>
            </div>

            <div className="form-group">
              <label>Value</label>
              <input
                type="number"
                min="1"
                value={value}
                onChange={(e) => setValue(e.target.value)}
                placeholder={operation === 'set' ? 'New score (e.g. 1000)' : 'Points (e.g. 50)'}
                disabled={submitting || !selectedPlayerId}
                required
              />
            </div>

            {error && <div className="error-message">⚠ {error}</div>}
            {success && <div className="success-message">✓ {success}</div>}

            <div className="form-actions">
              <button type="submit" className="btn-primary" disabled={submitting || !selectedPlayerId || !value}>
                {submitting ? 'Processing...' : `Apply ${operation === 'add' ? 'Addition' : operation === 'deduct' ? 'Deduction' : 'Change'}`}
              </button>
            </div>
          </form>

          <div className="undo-section">
            <button type="button" className="btn-undo" onClick={handleUndo} disabled={submitting}>
              ↩ Undo Last Operation
            </button>
            <p className="undo-hint">Reverts the most recent score change (add, deduct, or set)</p>
          </div>
        </div>

        <div className="preview-card">
          <h3>Selected Player Preview</h3>
          {selectedPlayer ? (
            <div className="player-preview">
              <div className="preview-header">
                <div className="preview-name">{selectedPlayer.name}</div>
                <div className="preview-id">#{selectedPlayer.id}</div>
              </div>
              <div className="preview-stats">
                <div className="stat">
                  <span className="stat-label">Current Score</span>
                  <span className="stat-value score">{selectedPlayer.score.toLocaleString()}</span>
                </div>
                <div className="stat">
                  <span className="stat-label">Current Rank</span>
                  <span className="stat-value rank">
                    <span className={`rank-badge ${getRankClass(players.indexOf(selectedPlayer) + 1)}`}>
                      #{players.indexOf(selectedPlayer) + 1}
                    </span>
                  </span>
                </div>
                <div className="stat">
                  <span className="stat-label">Created</span>
                  <span className="stat-value">{formatDate(selectedPlayer.created_at)}</span>
                </div>
                <div className="stat">
                  <span className="stat-label">Last Updated</span>
                  <span className="stat-value">{formatDate(selectedPlayer.updated_at)}</span>
                </div>
              </div>
            </div>
          ) : (
            <div className="no-selection">
              <span className="icon">👤</span>
              <p>Select a player to see preview</p>
            </div>
          )}
        </div>
      </div>

      <div className="quick-actions-card">
        <h3>Quick Actions</h3>
        <div className="quick-actions">
          {players.slice(0, 10).map((p, idx) => (
            <div key={p.id} className="quick-player">
              <span className="qp-rank">
                <span className={`rank-badge ${getRankClass(idx + 1)}`}>{idx + 1}</span>
              </span>
              <span className="qp-name">{p.name}</span>
              <span className="qp-score">{p.score.toLocaleString()}</span>
              <button
                className="qp-btn"
                onClick={() => {
                  setSelectedPlayerId(p.id)
                  setOperation('add')
                  setValue('10')
                }}
                title="Add 10 points"
              >
                +10
              </button>
              <button
                className="qp-btn"
                onClick={() => {
                  setSelectedPlayerId(p.id)
                  setOperation('add')
                  setValue('100')
                }}
                title="Add 100 points"
              >
                +100
              </button>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}

function getRankClass(rank) {
  if (rank === 1) return 'gold'
  if (rank === 2) return 'silver'
  if (rank === 3) return 'bronze'
  return ''
}

function formatDate(timestamp) {
  return new Date(timestamp * 1000).toLocaleString()
}