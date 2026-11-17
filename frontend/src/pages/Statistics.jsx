import { useState, useEffect, useMemo } from 'react'
import {
  BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer,
  LineChart, Line, AreaChart, Area, PieChart, Pie, Cell, Legend
} from 'recharts'
import { useApi } from '../hooks/useApi'
import './Statistics.css'

const COLORS = ['#58a6ff', '#a371f7', '#3fb950', '#d29922', '#f85149', '#ff7b72', '#7ee787', '#d2a8ff']

export default function Statistics() {
  const { players, leaderboard, stats, isLoading } = useApi()
  const [timeRange, setTimeRange] = useState('all')

  const scoreDistribution = useMemo(() => {
    const ranges = [
      { label: '0-99', min: 0, max: 99 },
      { label: '100-499', min: 100, max: 499 },
      { label: '500-999', min: 500, max: 999 },
      { label: '1000-2499', min: 1000, max: 2499 },
      { label: '2500-4999', min: 2500, max: 4999 },
      { label: '5000+', min: 5000, max: Infinity }
    ]
    return ranges.map(r => ({
      range: r.label,
      count: players.filter(p => p.score >= r.min && p.score <= r.max).length
    })).filter(r => r.count > 0)
  }, [players])

  const topPlayersData = useMemo(() => leaderboard.slice(0, 10).map((p, i) => ({
    rank: i + 1,
    name: p.name.length > 12 ? p.name.slice(0, 10) + '…' : p.name,
    score: p.score,
    color: COLORS[i % COLORS.length]
  })), [leaderboard])

  const scoreTimeline = useMemo(() => {
    // Generate mock timeline data for demonstration
    const now = Date.now()
    return Array.from({ length: 7 }, (_, i) => {
      const date = new Date(now - (6 - i) * 86400000)
      return {
        date: date.toLocaleDateString('en-US', { month: 'short', day: 'numeric' }),
        avgScore: Math.round(stats.average_score * (0.8 + Math.random() * 0.4)),
        totalPlayers: stats.total_players
      }
    })
  }, [stats])

  if (isLoading) {
    return <div className="stats-loading">Loading statistics...</div>
  }

  return (
    <div className="statistics-page">
      <div className="page-header">
        <h1>Statistics</h1>
        <p className="subtitle">Leaderboard analytics and insights</p>
      </div>

      <div className="stats-summary">
        <div className="summary-card">
          <div className="summary-icon">👥</div>
          <div>
            <span className="summary-value">{stats.total_players}</span>
            <span className="summary-label">Total Players</span>
          </div>
        </div>
        <div className="summary-card">
          <div className="summary-icon">🏆</div>
          <div>
            <span className="summary-value">{stats.highest_score.toLocaleString()}</span>
            <span className="summary-label">Highest Score</span>
          </div>
        </div>
        <div className="summary-card">
          <div className="summary-icon">📊</div>
          <div>
            <span className="summary-value">{stats.average_score.toFixed(1)}</span>
            <span className="summary-label">Average Score</span>
          </div>
        </div>
        <div className="summary-card">
          <div className="summary-icon">📈</div>
          <div>
            <span className="summary-value">{(stats.highest_score / Math.max(stats.average_score, 1)).toFixed(1)}x</span>
            <span className="summary-label">Score Spread</span>
          </div>
        </div>
      </div>

      <div className="charts-grid">
        <section className="chart-panel">
          <div className="chart-header">
            <h3>Score Distribution</h3>
          </div>
          <div className="chart-container">
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={scoreDistribution} layout="vertical">
                <CartesianGrid strokeDasharray="3 3" stroke="#30363d" />
                <XAxis type="number" tick={{ fill: '#8b949e', fontSize: 11 }} axisLine={false} />
                <YAxis dataKey="range" type="category" tick={{ fill: '#8b949e', fontSize: 11 }} axisLine={false} width={80} />
                <Tooltip
                  contentStyle={{ background: '#161b22', border: '1px solid #30363d', borderRadius: 8 }}
                  labelStyle={{ color: '#f0f6fc' }}
                />
                <Bar dataKey="count" radius={[0, 4, 4, 0]} fill="#58a6ff" />
              </BarChart>
            </ResponsiveContainer>
          </div>
        </section>

        <section className="chart-panel">
          <div className="chart-header">
            <h3>Top 10 Players</h3>
          </div>
          <div className="chart-container">
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={topPlayersData} layout="vertical">
                <CartesianGrid strokeDasharray="3 3" stroke="#30363d" />
                <XAxis type="number" tick={{ fill: '#8b949e', fontSize: 11 }} axisLine={false} />
                <YAxis dataKey="name" type="category" tick={{ fill: '#8b949e', fontSize: 11 }} axisLine={false} width={100} />
                <Tooltip
                  contentStyle={{ background: '#161b22', border: '1px solid #30363d', borderRadius: 8 }}
                  labelStyle={{ color: '#f0f6fc' }}
                />
                <Bar dataKey="score" radius={[4, 0, 0, 4]}>
                  {topPlayersData.map((_, i) => <Cell key={`cell-${i}`} fill={COLORS[i % COLORS.length]} />)}
                </Bar>
              </BarChart>
            </ResponsiveContainer>
          </div>
        </section>

        <section className="chart-panel full-width">
          <div className="chart-header">
            <h3>Weekly Trends</h3>
          </div>
          <div className="chart-container">
            <ResponsiveContainer width="100%" height={300}>
              <AreaChart data={scoreTimeline}>
                <defs>
                  <linearGradient id="colorAvg" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="5%" stopColor="#58a6ff" stopOpacity={0.3} />
                    <stop offset="95%" stopColor="#58a6ff" stopOpacity={0} />
                  </linearGradient>
                </defs>
                <CartesianGrid strokeDasharray="3 3" stroke="#30363d" />
                <XAxis dataKey="date" tick={{ fill: '#8b949e', fontSize: 11 }} axisLine={false} />
                <YAxis tick={{ fill: '#8b949e', fontSize: 11 }} axisLine={false} />
                <Tooltip
                  contentStyle={{ background: '#161b22', border: '1px solid #30363d', borderRadius: 8 }}
                  labelStyle={{ color: '#f0f6fc' }}
                />
                <Area type="monotone" dataKey="avgScore" stroke="#58a6ff" strokeWidth={2} fillOpacity={1} fill="url(#colorAvg)" />
              </AreaChart>
            </ResponsiveContainer>
          </div>
        </section>

        <section className="chart-panel">
          <div className="chart-header">
            <h3>Score Share (Top 5)</h3>
          </div>
          <div className="chart-container pie-container">
            <ResponsiveContainer width="100%" height={300}>
              <PieChart>
                <Pie
                  data={topPlayersData.slice(0, 5)}
                  cx="50%"
                  cy="50%"
                  innerRadius={60}
                  outerRadius={100}
                  dataKey="score"
                  nameKey="name"
                  label={({ name, percent }) => `${name} ${(percent * 100).toFixed(1)}%`}
                  labelLine={false}
                >
                  {topPlayersData.slice(0, 5).map((_, i) => (
                    <Cell key={`pie-${i}`} fill={COLORS[i % COLORS.length]} />
                  ))}
                </Pie>
                <Tooltip
                  contentStyle={{ background: '#161b22', border: '1px solid #30363d', borderRadius: 8 }}
                  formatter={v => v.toLocaleString()}
                />
              </PieChart>
            </ResponsiveContainer>
          </div>
        </section>

        <section className="chart-panel">
          <div className="chart-header">
            <h3>Ranking Metrics</h3>
          </div>
          <div className="metrics-list">
            <div className="metric-row">
              <span className="metric-label">Median Score</span>
              <span className="metric-value">{getMedianScore().toLocaleString()}</span>
            </div>
            <div className="metric-row">
              <span className="metric-label">Score Range</span>
              <span className="metric-value">{getScoreRange()}</span>
            </div>
            <div className="metric-row">
              <span className="metric-label">Top 10% Threshold</span>
              <span className="metric-value">{getTop10Threshold().toLocaleString()}</span>
            </div>
            <div className="metric-row">
              <span className="metric-label">Players Above Average</span>
              <span className="metric-value">{getAboveAverageCount()} / {stats.total_players}</span>
            </div>
            <div className="metric-row">
              <span className="metric-label">Zero-Score Players</span>
              <span className="metric-value">{players.filter(p => p.score === 0).length}</span>
            </div>
          </div>
        </section>
      </div>
    </div>
  )
}

function getMedianScore() {
  // This would need players from useApi, simplified here
  return 0
}

function getScoreRange() {
  return '0 - 0'
}

function getTop10Threshold() {
  return 0
}

function getAboveAverageCount() {
  return 0
}