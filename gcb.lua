-- Cache CPU info once at startup
gcb.CpuInfo = gcb.getCPUInfo()

print(string.format("CPU: %s", gcb.CpuInfo.name))
print(string.format("CPU: Threads: %d, CCDs: %d", gcb.CpuInfo.threads, gcb.CpuInfo.numCcds))

for i, ccd in ipairs(gcb.CpuInfo.ccds) do
  print(string.format(
    "CPU: CCD %d - Cores: %d, Threads: %d, X3D: %s, Thread Range: %d-%d",
    i - 1, ccd.cores, ccd.threads, tostring(ccd.isX3D), ccd.firstThread, ccd.lastThread
  ))
end

-- Checks if a directory exists via rename-trick
function gcb.dirExists(path)
  local ok, _, code = os.rename(path, path)
  if not ok then
    if code == 13 then -- Permission denied but exists
      return true
    end
    return false
  end
  return true
end

-- Ensures thread affinity only updated when different
function gcb.setProcessThreadsIfDifferent(pid, targetThreads)
  local currentThreads = gcb.getProcessThreads(pid)

  if #currentThreads == 0 then
    return -- PID likely no longer exists, skip
  end

  local function listsDiffer(a, b)
    if #a ~= #b then return true end
    local lookup = {}
    for _, v in ipairs(b) do lookup[v] = true end
    for _, v in ipairs(a) do if not lookup[v] then return true end end
    return false
  end

  if listsDiffer(targetThreads, currentThreads) then
    print("Thread affinity differs, updating...")
    print("Current threads: " .. table.concat(currentThreads, ", "))
    print("Target threads: " .. table.concat(targetThreads, ", "))
    local success = gcb.bindProcessToThreads(pid, targetThreads)
    if success then
      print("Affinity successfully set")
    else
      print("Failed to set affinity")
    end
  end
end

-- Applies thread affinity based on game settings
function gcb.setGameThreads(pid, settings)
  local mode = settings.Mode or "STANDARD"
  local smt = settings.SMT
  local targetThreads = {}

  local function addThreadRange(first, last, cores, threads, includeSMT)
    local threadsPerCore = threads / cores
    for t = first, last do
      if includeSMT or ((t - first) % threadsPerCore) == 0 then
        table.insert(targetThreads, t)
      end
    end
  end

  if mode == "STANDARD" then
    for _, ccd in ipairs(gcb.CpuInfo.ccds) do
      addThreadRange(ccd.firstThread, ccd.lastThread, ccd.cores, ccd.threads, true)
    end
    gcb.setProcessThreadsIfDifferent(pid, targetThreads)
    return
  end

  for i, ccd in ipairs(gcb.CpuInfo.ccds) do
    if (mode == "X3D" and ccd.isX3D) or (mode == "NON-X3D" and not ccd.isX3D) then
      addThreadRange(ccd.firstThread, ccd.lastThread, ccd.cores, ccd.threads, smt ~= false)
      break
    end
  end

  if mode == "X3D" and #targetThreads == 0 and #gcb.CpuInfo.ccds > 0 then
    local fallback = gcb.CpuInfo.ccds[1]
    addThreadRange(fallback.firstThread, fallback.lastThread, fallback.cores, fallback.threads, smt ~= false)
  end

  if #targetThreads > 0 then
    gcb.setProcessThreadsIfDifferent(pid, targetThreads)
  else
    print("No valid threads found for game settings, skipping affinity")
  end
end

-- Applies game affinity based on settings
function gcb.setGameCpuAffinity(gamePid, gameName)
  if not gamePid or not gameName then
    print("No valid game data, skipping CPU affinity")
    return
  end

  local gameData = getGame(gameName)
  if not gameData then
    print("No game settings found for: " .. gameName)
    return
  end

  local binding = gameData["Core-Binding"] or {}
  gcb.setGameThreads(gamePid, { Mode = binding.Mode or "STANDARD", SMT = binding.SMT })
end

-- Saves monitor states for later restoration
gcb.MonitorStates = {}

function gcb.saveMonitorStates()
  gcb.MonitorStates = gcb.getMonitors()
end

function gcb.disableNonPrimaryMonitors()
  local monitors = gcb.getMonitors()
  for _, m in ipairs(monitors) do
    if not m.isPrimary then
      gcb.disableMonitor(m.device)
    end
  end
end

function gcb.enableNonPrimaryMonitors()
  for _, m in ipairs(gcb.MonitorStates) do
    if not m.isPrimary then
      gcb.enableMonitor(m)
    end
  end
end

-- Custom LUA code file

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

function gcb.reloadCustomLuaIfChanged()
  local ts = gcb.getFileTimestamp(customFile)
  if ts > 0 and ts ~= customTimestamp then
    print(customFile .. " changed, reloading...")
    gcb.loadCustomLua()
  end
end
