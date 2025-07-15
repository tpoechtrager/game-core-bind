-- Load version file
require("version")

-- Cache CPU info once at startup
gcb.CpuInfo = gcb.getCPUInfo()

-- Print info on startup

print(string.format("Game Core Bind - Version %d (%s)", gcb.version.Build, gcb.version.GitRev))
print(string.format("CPU: %s", gcb.CpuInfo.name))
print(string.format("CPU: Threads: %d, CCDs: %d", gcb.CpuInfo.threads, gcb.CpuInfo.numCcds))

for i, ccd in ipairs(gcb.CpuInfo.ccds) do
  print(string.format(
    "CPU: CCD %d - Cores: %d, Threads: %d, X3D: %s, Thread Range: %d-%d",
    i - 1, ccd.cores, ccd.threads, tostring(ccd.isX3D), ccd.firstThread, ccd.lastThread
  ))
end

-- Process Thread binding
gcb.SET_PROCESS_THREADS_SUCCESS = 0
gcb.SET_PROCESS_THREADS_ERROR = 1
gcb.SET_PROCESS_THREADS_PERMISSION_DENIED = 2

function gcb.setProcessThreadsIfDifferent(pid, targetThreads)
  local result = gcb.getProcessThreads(pid)

  if result.code ~= gcb.PROCESS_GET_THREADS_SUCCESS then
    if result.code == gcb.PROCESS_GET_THREADS_PERMISSION_DENIED then
      print(string.format("setProcessThreadsIfDifferent: Permission denied for PID %d", pid))
      return gcb.SET_PROCESS_THREADS_PERMISSION_DENIED
    end
    print(string.format("setProcessThreadsIfDifferent: Failed to query threads for PID %d (Code: %d)", pid, result.code))
    return gcb.SET_PROCESS_THREADS_ERROR
  end

  local currentThreads = result.threads

  local function listsDiffer(a, b)
    if #a ~= #b then return true end
    local lookup = {}
    for _, v in ipairs(b) do lookup[v] = true end
    for _, v in ipairs(a) do if not lookup[v] then return true end end
    return false
  end

  if listsDiffer(targetThreads, currentThreads) then
    print("setProcessThreadsIfDifferent: Thread affinity differs, updating...")
    print("setProcessThreadsIfDifferent: Current threads: " .. table.concat(currentThreads, ", "))
    print("setProcessThreadsIfDifferent: Target threads: " .. table.concat(targetThreads, ", "))

    local code = gcb.bindProcessToThreads(pid, targetThreads)
    if code == gcb.PROCESS_BIND_PERMISSION_DENIED then
      print(string.format("setProcessThreadsIfDifferent: Permission denied for PID %d", pid))
      return gcb.SET_PROCESS_THREADS_PERMISSION_DENIED
    elseif code ~= gcb.PROCESS_BIND_SUCCESS then
      print(string.format("setProcessThreadsIfDifferent: Failed to set affinity for PID %d, code: %d", pid, code))
      return gcb.SET_PROCESS_THREADS_ERROR
    end

    print("setProcessThreadsIfDifferent: Affinity successfully set")
  end

  return gcb.SET_PROCESS_THREADS_SUCCESS
end

-- Applies thread affinity based on game settings
gcb.CoreBindingMode = {
  STANDARD = "STANDARD",
  X3D = "X3D",
  NON_X3D = "NON-X3D"
}

gcb.SET_GAME_THREADS_SUCCESS = 0
gcb.SET_GAME_THREADS_ERROR = 1
gcb.SET_GAME_THREADS_PERMISSION_DENIED = 2

function gcb.setGameThreads(pid, settings)
  local mode = settings.Mode or gcb.CoreBindingMode.STANDARD
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

  if mode == gcb.CoreBindingMode.STANDARD then
    for _, ccd in ipairs(gcb.CpuInfo.ccds) do
      addThreadRange(ccd.firstThread, ccd.lastThread, ccd.cores, ccd.threads, true)
    end
  else
    for _, ccd in ipairs(gcb.CpuInfo.ccds) do
      if (mode == gcb.CoreBindingMode.X3D and ccd.isX3D) or
         (mode == gcb.CoreBindingMode.NON_X3D and not ccd.isX3D) then
        addThreadRange(ccd.firstThread, ccd.lastThread, ccd.cores, ccd.threads, smt ~= false)
        break
      end
    end

    if mode == gcb.CoreBindingMode.X3D and #targetThreads == 0 and #gcb.CpuInfo.ccds > 0 then
      local fallback = gcb.CpuInfo.ccds[1]
      addThreadRange(fallback.firstThread, fallback.lastThread, fallback.cores, fallback.threads, smt ~= false)
    end
  end

  if #targetThreads == 0 then
    print("setGameThreads: No valid threads found, skipping")
    return gcb.SET_GAME_THREADS_ERROR
  end

  local code = gcb.setProcessThreadsIfDifferent(pid, targetThreads)
  if code == gcb.SET_PROCESS_THREADS_PERMISSION_DENIED then
    return gcb.SET_GAME_THREADS_PERMISSION_DENIED
  elseif code == gcb.SET_PROCESS_THREADS_SUCCESS then
    return gcb.SET_GAME_THREADS_SUCCESS
  else
    return gcb.SET_GAME_THREADS_ERROR
  end
end

-- Applies game affinity based on settings
gcb.SET_GAME_CPU_AFFINITY_SUCCESS = 0
gcb.SET_GAME_CPU_AFFINITY_ERROR = 1
gcb.SET_GAME_CPU_AFFINITY_PERMISSION_DENIED = 2

function gcb.setGameCpuAffinity(gamePid, gameName)
  if not gamePid or not gameName then
    print("setGameCpuAffinity: Invalid parameters")
    return gcb.SET_GAME_CPU_AFFINITY_ERROR
  end

  local gameData = getGame(gameName)
  if not gameData then
    print("setGameCpuAffinity: No settings found for: " .. gameName)
    return gcb.SET_GAME_CPU_AFFINITY_ERROR
  end

  local binding = gameData["Core-Binding"] or {}
  local code = gcb.setGameThreads(gamePid, { Mode = binding.Mode or "STANDARD", SMT = binding.SMT })

  if code == gcb.SET_GAME_THREADS_PERMISSION_DENIED then
    return gcb.SET_GAME_CPU_AFFINITY_PERMISSION_DENIED
  elseif code == gcb.SET_GAME_THREADS_SUCCESS then
    return gcb.SET_GAME_CPU_AFFINITY_SUCCESS
  else
    return gcb.SET_GAME_CPU_AFFINITY_ERROR
  end
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

-- Writes current Config table to config.lua
gcb.saveConfig = function()
  local file = io.open("config.lua", "w")
  if not file then return false end

  file:write("-- This file is generated automatically by GCB. Do not edit manually.\n")
  file:write("Config = {\n")
  for k, v in pairs(Config) do
    file:write(string.format("  %s = %s,\n", k, tostring(v)))
  end
  file:write("}\n")
  file:close()
  return true
end
