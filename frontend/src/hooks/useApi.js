import { useState, useCallback, useEffect } from 'react'

const API_BASE = '/api'

async function request(endpoint, options = {}) {
  const res = await fetch(`${API_BASE}${endpoint}`, {
    headers: { 'Content-Type': 'application/json', ...options.headers },
    ...options
  })
  if (!res.ok) {
    const err = await res.json().catch(() => ({ message: 'Request failed' }))
    throw new Error(err.error?.message || err.message || `HTTP ${res.status}`)
  }
  return res.json()
}

export function useApi() {
  const [players, setPlayers] = useState([])
  const [leaderboard, setLeaderboard] = useState([])
  const [stats, setStats] = useState({ total_players: 0, highest_score: 0, average_score: 0 })
  const [activity, setActivity] = useState([])
  const [isLoading, setIsLoading] = useState(false)
  const [error, setError] = useState(null)

  const fetchAll = useCallback(async () => {
    try {
      setIsLoading(true)
      setError(null)
      const [p, lb, s, a] = await Promise.all([
        request('/players?limit=100'),
        request('/leaderboard?limit=100'),
        request('/stats'),
        request('/activity?limit=50')
      ])
      setPlayers(p)
      setLeaderboard(lb)
      setStats(s)
      setActivity(a)
    } catch (e) {
      setError(e.message)
    } finally {
      setIsLoading(false)
    }
  }, [])

  const api = {
    players: {
      list: () => request('/players?limit=100'),
      get: (id) => request(`/players/${id}`),
      create: (name) => request('/players', { method: 'POST', body: JSON.stringify({ name }) }),
      delete: (id) => request(`/players/${id}`, { method: 'DELETE' }),
      search: (q) => request(`/search?q=${encodeURIComponent(q)}`)
    },
    scores: {
      add: (id, value) => request('/score/add', { method: 'POST', body: JSON.stringify({ id, value }) }),
      deduct: (id, value) => request('/score/deduct', { method: 'POST', body: JSON.stringify({ id, value }) }),
      set: (id, value) => request('/score/set', { method: 'POST', body: JSON.stringify({ id, value }) }),
      undo: () => request('/score/undo', { method: 'POST' })
    },
    leaderboard: {
      get: (limit = 10) => request(`/leaderboard?limit=${limit}`)
    },
    stats: {
      get: () => request('/stats')
    },
    activity: {
      get: (limit = 50) => request(`/activity?limit=${limit}`)
    }
  }

  useEffect(() => { fetchAll() }, [fetchAll])

  return { players, leaderboard, stats, activity, isLoading, error, api, refreshAll: fetchAll }
}

export function usePlayer(id) {
  const [player, setPlayer] = useState(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState(null)

  useEffect(() => {
    if (!id) return
    setLoading(true)
    request(`/players/${id}`)
      .then(setPlayer)
      .catch(setError)
      .finally(() => setLoading(false))
  }, [id])

  return { player, loading, error, refetch: () => request(`/players/${id}`).then(setPlayer) }
}