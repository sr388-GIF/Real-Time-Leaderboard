import { Outlet, NavLink, useLocation } from 'react-router-dom'
import { useState } from 'react'
import './Layout.css'

const navItems = [
  { path: '/dashboard', label: 'Dashboard', icon: '📊' },
  { path: '/leaderboard', label: 'Leaderboard', icon: '🏆' },
  { path: '/players', label: 'Players', icon: '👥' },
  { path: '/scores', label: 'Update Scores', icon: '⚡' },
  { path: '/activity', label: 'Activity', icon: '📜' },
  { path: '/statistics', label: 'Statistics', icon: '📈' }
]

export default function Layout() {
  const [sidebarOpen, setSidebarOpen] = useState(true)
  const location = useLocation()

  return (
    <div className="layout">
      <aside className={`sidebar ${sidebarOpen ? 'open' : 'collapsed'}`}>
        <div className="sidebar-header">
          <h1 className="logo">🏆 Leaderboard</h1>
          <button
            className="sidebar-toggle"
            onClick={() => setSidebarOpen(!sidebarOpen)}
            aria-label={sidebarOpen ? 'Collapse sidebar' : 'Expand sidebar'}
          >
            {sidebarOpen ? '◀' : '▶'}
          </button>
        </div>
        <nav className="sidebar-nav">
          {navItems.map(item => (
            <NavLink
              key={item.path}
              to={item.path}
              className={({ isActive }) => `nav-item ${isActive ? 'active' : ''}`}
            >
              <span className="nav-icon">{item.icon}</span>
              <span className="nav-label">{item.label}</span>
            </NavLink>
          ))}
        </nav>
        <div className="sidebar-footer">
          <div className="version">v1.0.0</div>
        </div>
      </aside>

      <main className="main-content">
        <header className="top-bar">
          <button className="mobile-menu-btn" onClick={() => setSidebarOpen(true)}>
            ☰
          </button>
          <div className="page-title">
            <h2>{getPageTitle(location.pathname)}</h2>
          </div>
          <div className="top-bar-actions">
            <div className="connection-status">
              <span className="status-dot online"></span>
              <span>Live</span>
            </div>
          </div>
        </header>
        <div className="content-area">
          <Outlet />
        </div>
      </main>

      {sidebarOpen && window.innerWidth < 1024 && (
        <div className="sidebar-overlay" onClick={() => setSidebarOpen(false)} />
      )}
    </div>
  )
}

function getPageTitle(path) {
  const titles = {
    '/dashboard': 'Dashboard',
    '/leaderboard': 'Leaderboard',
    '/players': 'Player Management',
    '/players/add': 'Add Player',
    '/scores': 'Update Scores',
    '/activity': 'Activity History',
    '/statistics': 'Statistics'
  }
  return titles[path] || 'Leaderboard'
}