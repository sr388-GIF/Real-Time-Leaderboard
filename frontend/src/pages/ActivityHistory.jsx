import { useState, useEffect } from 'react'
import { useApi } from '../hooks/useApi'
import './ActivityHistory.css'

export default function ActivityHistory() {
  const { activity, isLoading, api, refreshAll } = useApi()
  const [filterOp, setFilterOp] = useState('all')
  const [filterPlayer, setFilterPlayer] = useState('')
  const [page, setPage] = useState(1)
  const pageSize = 20

  const operations = [
    { value: 'all', label: 'All Operations' },
    { value: 'add_points', label: 'Add Points' },
    { value: 'deduct_points', label: 'Deduct Points' },
    { value: 'set_score', label: 'Set Score' },
    { value: 'create_player', label: 'Create Player' },
    { value: 'delete_player', label: 'Delete Player' }
  ]

  const filteredActivity = activity.filter(a => {
    if (filterOp !== 'all' && a.op_type !== filterOp) return false
    if (filterPlayer && !a.player_name.toLowerCase().includes(filterPlayer.toLowerCase())) return false
    return true
  })

  const totalPages = Math.ceil(filteredActivity.length / pageSize)
  const paginatedActivity = filteredActivity.slice((page - 1) * pageSize, page * pageSize)

  return (
    <div className="activity-page">
      <div className="page-header">
        <h1>Activity History</h1>
        <p className="subtitle">Chronological log of all leaderboard changes</p>
      </div>

      <div className="filters-bar">
        <div className="filter-group">
          <label>Operation</label>
          <select value={filterOp} onChange={e => setFilterOp(e.target.value)}>
            {operations.map(op => (
              <option key={op.value} value={op.value}>{op.label}</option>
            ))}
          </select>
        </div>
        <div className="filter-group">
          <label>Player</label>
          <input
            type="text"
            placeholder="Filter by player name..."
            value={filterPlayer}
            onChange={e => setFilterPlayer(e.target.value)}
          />
        </div>
        <div className="filter-group">
          <label>Results</label>
          <span className="results-count">
            {filteredActivity.length} {filteredActivity.length === 1 ? 'entry' : 'entries'}
          </span>
        </div>
      </div>

      <div className="activity-table-wrapper">
        {isLoading ? (
          <div className="loading">Loading activity...</div>
        ) : (
          <table className="activity-table">
            <thead>
              <tr>
                <th>Time</th>
                <th>Player</th>
                <th>Operation</th>
                <th>Score Change</th>
                <th>Before → After</th>
              </tr>
            </thead>
            <tbody>
              {paginatedActivity.map((entry, idx) => (
                <tr key={idx}>
                  <td className="time">{formatDateTime(entry.timestamp)}</td>
                  <td>
                    <div className="player-cell">
                      <span className="player-name">{entry.player_name}</span>
                      <span className="player-id">#{entry.player_id}</span>
                    </div>
                  </td>
                  <td>
                    <span className={`op-badge ${getOpClass(entry.op_type)}`}>
                      {getOpIcon(entry.op_type)} {getOpLabel(entry.op_type)}
                    </span>
                  </td>
                  <td className="change {getChangeClass(entry.op_type, entry.old_score, entry.new_score)}">
                    {getChangeText(entry.op_type, entry.old_score, entry.new_score)}
                  </td>
                  <td className="score-transition">
                    <span className="old-score">{entry.old_score.toLocaleString()}</span>
                    <span className="arrow">→</span>
                    <span className="new-score">{entry.new_score.toLocaleString()}</span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>

      {filteredActivity.length === 0 && !isLoading && (
        <div className="empty-state">
          <div className="empty-icon">📜</div>
          <h3>No activity found</h3>
          <p>{filterOp !== 'all' || filterPlayer ? 'Try adjusting your filters' : 'No activity recorded yet'}</p>
          {(filterOp !== 'all' || filterPlayer) && (
            <button onClick={() => { setFilterOp('all'); setFilterPlayer('') }} className="btn-secondary">
              Clear Filters
            </button>
          )}
        </div>
      )}

      {totalPages > 1 && (
        <div className="pagination">
          <button className="page-btn" onClick={() => setPage(p => Math.max(1, p - 1))} disabled={page === 1}>
            ← Previous
          </button>
          <span className="page-info">Page {page} of {totalPages}</span>
          <button className="page-btn" onClick={() => setPage(p => Math.min(totalPages, p + 1))} disabled={page === totalPages}>
            Next →
          </button>
        </div>
      )}
    </div>
  )
}

function getOpClass(opType) {
  switch (opType) {
    case 'add_points': return 'add'
    case 'deduct_points': return 'deduct'
    case 'set_score': return 'set'
    case 'create_player': return 'create'
    case 'delete_player': return 'delete'
    default: return ''
  }
}

function getOpIcon(opType) {
  switch (opType) {
    case 'add_points': return '+'
    case 'deduct_points': return '−'
    case 'set_score': return '='
    case 'create_player': return '+'
    case 'delete_player': return '🗑'
    default: return '•'
  }
}

function getOpLabel(opType) {
  switch (opType) {
    case 'add_points': return 'Add Points'
    case 'deduct_points': return 'Deduct Points'
    case 'set_score': return 'Set Score'
    case 'create_player': return 'Create Player'
    case 'delete_player': return 'Delete Player'
    default: return opType
  }
}

function getChangeClass(opType, oldScore, newScore) {
  if (opType === 'add_points' || (opType === 'set_score' && newScore > oldScore)) return 'positive'
  if (opType === 'deduct_points' || (opType === 'set_score' && newScore < oldScore)) return 'negative'
  return 'neutral'
}

function getChangeText(opType, oldScore, newScore) {
  if (opType === 'create_player') return 'Created'
  if (opType === 'delete_player') return 'Deleted'
  if (opType === 'add_points') return `+${newScore - oldScore}`
  if (opType === 'deduct_points') return `-${oldScore - newScore}`
  if (opType === 'set_score') return newScore > oldScore ? `+${newScore - oldScore}` : `-${oldScore - newScore}`
  return ''
}

function formatDateTime(timestamp) {
  const date = new Date(timestamp * 1000)
  return date.toLocaleString()
}

function formatDate(timestamp) {
  return new Date(timestamp * 1000).toLocaleDateString()
}