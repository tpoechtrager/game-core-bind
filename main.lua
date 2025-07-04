if Config.EnsureRunningAsAdmin and not gcb.isRunningAsAdmin() then
  print("Not running as admin, restarting with elevated privileges...")
  gcb.restartAsAdmin()
  return
end

-- Initial load of Custom LUA
gcb.loadCustomLua()

gcb.currentGames = {}

local askedForAdmin = false

gcb.onTick = function()
  if Config.SetCpuAffinity then
    for _, game in ipairs(gcb.currentGames) do
      gcb.setGameCpuAffinity(game.pid, game.name)
    end
  end

  gcb.reloadCustomLuaIfChanged()
end

local askedForAdmin = false

gcb.onGameStart = function(pid, name, binary)
  table.insert(gcb.currentGames, {
    pid = pid,
    name = name,
    binary = binary
  })

  print("Game started: " .. name .. " (" .. binary .. "), PID: " .. pid)

  local gameData = getGame(name)
  if gameData and gameData["Init-Wait"] and gameData["Init-Wait"].WaitMs then
    print("Sleeping " .. gameData["Init-Wait"].WaitMs .. " ms")
    gcb.sleepMs(gameData["Init-Wait"].WaitMs)
  end

  -- Always set affinity, even if the same game runs multiple times
  if Config.SetCpuAffinity then
    local code = gcb.setGameCpuAffinity(pid, name)
    if code == gcb.SET_GAME_CPU_AFFINITY_PERMISSION_DENIED and not askedForAdmin then
      askedForAdmin = true
      if gcb.showYesNoBox("GCB: Permission Denied", 
        string.format("Failed to set CPU affinity for '%s'.\nRestart GCB as admin?", name)) then
        gcb.restartAsAdmin()
      end
    end
  end

  -- Prevent duplicate handling if multiple instances are detected
  if #gcb.currentGames >= 2 then
    return
  end

  if Config.DisableDesktopEffects then
    gcb.disableDesktopEffects()
  end

  if Config.DisableNonPrimaryDisplays then
    print("Saving and disabling non-primary monitors...")
    gcb.saveMonitorStates()
    gcb.disableNonPrimaryMonitors()
  end

  if custom and type(custom.gameStart) == "function" then
    custom.gameStart(pid, name, binary)
  end
end


gcb.onGameStop = function(pid, name, binary)
  print("Game stopped: " .. name .. " (" .. binary .. "), PID: " .. pid)

  -- Only handle the first instance of a game
  if not gcb.currentGames[1] or gcb.currentGames[1].pid ~= pid then
    for i, game in ipairs(gcb.currentGames) do
      if game.pid == pid then
        table.remove(gcb.currentGames, i)
        break
      end
    end
    return
  end

  table.remove(gcb.currentGames, 1)

  if Config.DisableNonPrimaryDisplays then
    print("Restoring monitor state...")
    gcb.enableNonPrimaryMonitors()
  end

  if Config.DisableDesktopEffects then
    gcb.enableDesktopEffects()
  end

  if custom and type(custom.gameStop) == "function" then
    custom.gameStop(pid, name, binary)
  end
end


gcb.onGameForeground = function(pid, name, binary)
  if custom and type(custom.gameForeground) == "function" then
    custom.gameForeground(pid, name, binary)
  end
end

gcb.onGameBackground = function(pid, name, binary)
  if custom and type(custom.gameBackground) == "function" then
    custom.gameBackground(pid, name, binary)
  end
end

print("Waiting for games to be launched ...")
