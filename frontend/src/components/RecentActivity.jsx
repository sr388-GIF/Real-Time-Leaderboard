import './RecentActivity.css'

export default function RecentActivity({ activities = [] }) {
  if (activities.length === 0) {
    return (
      <div className="activity-empty">
        <div className="empty-icon">📜</div>
        <p>No activity yet</p>
        <span>Score changes will appear here</span>
      </div>
    )
  }

  return (
    <div className="activity-list">
      {activities.map((activity, idx) => (
        <div key={idx} className="activity-item">
          <div className={`activity-icon ${getOpClass(activity.op_type)}`}>
            {getOpIcon(activity.op_type)}
          </div>
          <div className="activity-details">
            <div className="activity-main">
              <span className="activity-player">{activity.player_name}</span>
              <span className={`activity-change ${getChangeClass(activity.op_type, activity.old_score, activity.new_score)}`}>
                {getChangeText(activity.op_type, activity.old_score, activity.new_score)}
              </span>
            </div>
            <div className="activity-time">
              {formatTime(activity.timestamp)}
            </div>
          </div>
        </div>
      ))}
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

function getChangeClass(opType, oldScore, newScore) {
  if (opType === 'add_points' || (opType === 'set_score' && newScore > oldScore)) return 'positive'
  if (opType === 'deduct_points' || (opType === 'set_score' && newScore < oldScore)) return 'negative'
  return 'neutral'
}

function getChangeText(opType, oldScore, newScore) {
  if (opType === 'create_player') return 'Player created'
  if (opType === 'delete_player') return 'Player deleted'
  if (opType === 'add_points') return `+${newScore - oldScore} (${oldScore} → ${newScore})`
  if (opType === 'deduct_points') return `-${oldScore - newScore} (${oldScore} → ${newScore})`
  if (opType === 'set_score') return `${oldScore} → ${newScore}`
  return `${oldScore} → ${newScore}`
}

function formatTime(timestamp) {
  const date = new Date(timestamp * 1000)
  const now = new Date()
  const diff = now - date

  if (diff < 60000) return 'Just now'
  if (diff < 3600000) return `${Math.floor(diff / 60000)}m ago`
  if (diff < 86400000) return `${Math.floor(diff / 3600000)}h ago`
  return date.toLocaleDateString() + ' ' + date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
}