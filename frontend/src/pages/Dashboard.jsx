import { useApi } from '../hooks/useApi'
import StatCard from '../components/StatCard'
import TopPlayers from '../components/TopPlayers'
import RecentActivity from '../components/RecentActivity'
import './Dashboard.css'

export default function Dashboard() {
  const { stats, leaderboard, activity, isLoading } = useApi()

  return (
    <div className="dashboard">
      <div className="dashboard-header">
        <h1>Dashboard</h1>
        <p className="subtitle">Real-time overview of leaderboard statistics</p>
      </div>

      <div className="stats-grid">
        <StatCard
          label="Total Players"
          value={stats.total_players}
          icon="👥"
          trend="+12%"
          trendLabel="vs last week"
        />
        <StatCard
          label="Highest Score"
          value={stats.highest_score.toLocaleString()}
          icon="🏆"
          trend="New record"
          trendLabel="This season"
        />
        <StatCard
          label="Average Score"
          value={stats.average_score.toFixed(1)}
          icon="📊"
          trend="+5.2%"
          trendLabel="vs last week"
        />
        <StatCard
          label="Active Today"
          value={activity.filter(a => {
            const day = new Date(a.timestamp * 1000).toDateString()
            return day === new Date().toDateString()
          }).length}
          icon="⚡"
          trend="Live"
          trendLabel="Real-time"
        />
      </div>

      <div className="dashboard-grid">
        <section className="panel top-players-panel">
          <div className="panel-header">
            <h2>Top 3 Players</h2>
            <span className="panel-badge">Live</span>
          </div>
          <TopPlayers players={leaderboard.slice(0, 3)} />
        </section>

        <section className="panel recent-activity-panel">
          <div className="panel-header">
            <h2>Recent Activity</h2>
            <span className="panel-badge">Last 10</span>
          </div>
          <RecentActivity activities={activity.slice(0, 10)} />
        </section>
      </div>

      <section className="panel full-width leaderboard-preview">
        <div className="panel-header">
          <h2>Leaderboard Preview</h2>
          <a href="/leaderboard" className="view-all">View All →</a>
        </div>
        <div className="leaderboard-table-container">
          <table className="leaderboard-table">
            <thead>
              <tr>
                <th>Rank</th>
                <th>Player</th>
                <th>Score</th>
                <th>Last Updated</th>
              </tr>
            </thead>
            <tbody>
              {leaderboard.slice(0, 10).map((player, idx) => (
                <tr key={player.id} className={idx < 3 ? 'top-three' : ''}>
                  <td className="rank">
                    {getRankBadge(idx + 1)}
                  </td>
                  <td>
                    <div className="player-cell">
                      <span className="player-name">{player.name}</span>
                      <span className="player-id">#{player.id}</span>
                    </div>
                  </td>
                  <td className="score">{player.score.toLocaleString()}</td>
                  <td className="updated">{formatDate(player.updated_at)}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </section>
    </div>
  )
}

function getRankBadge(rank) {
  if (rank === 1) return <span className="rank-badge gold">🥇 1</span>
  if (rank === 2) return <span className="rank-badge silver">🥈 2</span>
  if (rank === 3) return <span className="rank-badge bronze">🥉 3</span>
  return <span className="rank-badge">{rank}</span>
}

function formatDate(timestamp) {
  const date = new Date(timestamp * 1000)
  return date.toLocaleString()
}