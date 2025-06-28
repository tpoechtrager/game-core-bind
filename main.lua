gcb.currentGame = nil

local customFile = "custom.lua"
local customTimestamp = 0

function gcb.loadCustomLua()
  local f = io.open(customFile, "r")
  if f then
    f:close()
    dofile(customFile)
    customTimestamp = gcb.getFileTimestamp(customFile)
    print("Loaded " .. customFile)
  end
end

-- Initial load
gcb.loadCustomLua()

gcb.onTick = function()
  if gcb.currentGame and Config.SetCpuAffinity then
    gcb.setCurrentGameCpuAffinity()
  end

  local ts = gcb.getFileTimestamp(customFile)

  if ts > 0 and ts ~= customTimestamp then
    print(customFile .. " changed, reloading...")
    gcb.loadCustomLua()
  end
end

gcb.onGameStart = function(pid, name, binary)
  print("Game started: " .. name .. " (" .. binary .. "), PID: " .. pid)

  -- Look up the game in the Games table to check for an initialization wait time
  local gameData = getGame(name)
  if gameData and gameData["Init-Wait"] and gameData["Init-Wait"].WaitMs then
    -- Sleep for the specified milliseconds before continuing
    print("Sleeping " .. gameData["Init-Wait"].WaitMs .. " ms")
    gcb.sleepMs(gameData["Init-Wait"].WaitMs)
  end

  gcb.currentGame = {
    pid = pid,
    name = name,
    binary = binary
  }

  if Config.DisableDesktopEffects then
    gcb.disableDesktopEffects()
  end

  if Config.DisableNonPrimaryDisplays then
    print("Saving and disabling non-primary monitors...")
    gcb.saveMonitorStates()
    gcb.disableNonPrimaryMonitors()
  end

  if Config.SetCpuAffinity then
    gcb.setCurrentGameCpuAffinity()
  end

  if custom and type(custom.gameStart) == "function" then
    custom.gameStart(pid, name, binary)
  end
end


gcb.onGameStop = function(pid, name, binary)
  print("Game stopped: " .. name .. " (" .. binary .. "), PID: " .. pid)

  gcb.currentGame = nil

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

if Config.HideConsole and not gcb.dirExists("src") then
  gcb.hideConsole()
end

print("Waiting for games to be launched ...")
