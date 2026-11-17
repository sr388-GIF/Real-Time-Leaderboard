import { useState, useEffect } from 'react'
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom'
import Layout from './components/Layout'
import Dashboard from './pages/Dashboard'
import Leaderboard from './pages/Leaderboard'
import PlayerManagement from './pages/PlayerManagement'
import AddPlayer from './pages/AddPlayer'
import UpdateScores from './pages/UpdateScores'
import PlayerDetails from './pages/PlayerDetails'
import ActivityHistory from './pages/ActivityHistory'
import Statistics from './pages/Statistics'
import { useApi } from './hooks/useApi'

function App() {
  const { refreshAll, isLoading } = useApi()

  useEffect(() => {
    refreshAll()
    const interval = setInterval(() => refreshAll(), 5000) // Poll every 5 seconds
    return () => clearInterval(interval)
  }, [refreshAll])

  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Layout />}>
          <Route index element={<Navigate to="/dashboard" replace />} />
          <Route path="dashboard" element={<Dashboard />} />
          <Route path="leaderboard" element={<Leaderboard />} />
          <Route path="players" element={<PlayerManagement />} />
          <Route path="players/add" element={<AddPlayer />} />
          <Route path="scores" element={<UpdateScores />} />
          <Route path="players/:id" element={<PlayerDetails />} />
          <Route path="activity" element={<ActivityHistory />} />
          <Route path="statistics" element={<Statistics />} />
        </Route>
      </Routes>
    </BrowserRouter>
  )
}

export default App