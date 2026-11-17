import './TopPlayers.css'

export default function TopPlayers({ players = [] }) {
  if (players.length === 0) {
    return (
      <div className="top-players-empty">
        <div className="empty-icon">🏆</div>
        <p>No players yet</p>
        <span>Add players to see rankings</span>
      </div>
    )
  }

  return (
    <div className="top-players-list">
      {players.map((player, idx) => (
        <div key={player.id} className="top-player-item">
          <div className={`top-player-rank ${idx === 0 ? 'gold' : idx === 1 ? 'silver' : 'bronze'}`}>
            {idx === 0 ? '🥇' : idx === 1 ? '🥈' : '🥉'}
          </div>
          <div className="top-player-info">
            <div className="top-player-name">{player.name}</div>
            <div className="top-player-id">#{player.id}</div>
          </div>
          <div className="top-player-score">{player.score.toLocaleString()}</div>
        </div>
      ))}
    </div>
  )
}