custom = {}

local function SetDesktopMouseSensitivity()
  print("Setting desktop mouse sensitivity...")
  gcb.runDetached("C:\\Users\\thomas\\Desktop\\mon\\mouse.exe", "--dpi 1550 --poll-rate 1000")
end

local function SetIngameMouseSensitivity(pid, name, binary)
  local dpi, pollRate
  if name == "Cyberpunk 2077" or name == "DOOM The Dark Ages" then
    dpi = 1550
    pollRate = 4000
  else
    dpi = 20000
    pollRate = 500
  end

  print("Setting ingame mouse sensitivity...")
  gcb.runDetached("C:\\Users\\thomas\\Desktop\\mon\\mouse.exe",
                  string.format("--dpi %d --poll-rate %d", dpi, pollRate))
end

function custom.gameStart(pid, name, binary)
  gcb.sendUdp("192.168.0.1", 8787, "game_start") -- Enables QoS on the router
end

function custom.gameStop(pid, name, binary)
  gcb.sendUdp("192.168.0.1", 8787, "game_stop") -- Disables QoS on the router
  SetDesktopMouseSensitivity()
end

function custom.gameForeground(pid, name, binary)
  SetIngameMouseSensitivity(pid, name, binary)
end

function custom.gameBackground(pid, name, binary)
  SetDesktopMouseSensitivity()
end
