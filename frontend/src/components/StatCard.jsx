import './StatCard.css'

export default function StatCard({ label, value, icon, trend, trendLabel }) {
  const isPositive = trend?.startsWith('+') || trend?.startsWith('New')
  const trendColor = isPositive ? 'var(--success)' : 'var(--danger)'

  return (
    <div className="stat-card animate-fade">
      <div className="stat-header">
        <div className="stat-icon">{icon}</div>
        <span className="stat-label">{label}</span>
      </div>
      <div className="stat-value">{value}</div>
      {trend && (
        <div className="stat-trend" style={{ color: trendColor }}>
          <span className="trend-value">{trend}</span>
          <span className="trend-label">{trendLabel}</span>
        </div>
      )}
    </div>
  )
}