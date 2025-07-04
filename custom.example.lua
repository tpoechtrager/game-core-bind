custom = {}

local function SetDesktopMouseSensitivity()
  print("Setting desktop mouse sensitivity...")
  os.execute([[C:\Users\thomas\Desktop\mon\mouse.exe --dpi 1550 --poll-rate 1000 > NUL 2>&1]])
end

local function SetIngameMouseSensitivity(pid, name, binary)
  local dpi, pollRate
  if name == "Cyberpunk 2077" then
    dpi = 1550
    pollRate = 4000
  else
    dpi = 20000
    pollRate = 500
  end

  print("Setting ingame mouse sensitivity...")
  os.execute(string.format([[C:\Users\thomas\Desktop\mon\mouse.exe --dpi %d --poll-rate %d > NUL 2>&1]], dpi, pollRate))
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
