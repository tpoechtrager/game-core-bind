custom = {}

local function SetDesktopMouseSensitivity()
  print("Setting desktop mouse sensitivity...")
  os.execute([[C:\Users\thomas\Desktop\mon\mouse.exe --dpi 1550 --poll-rate 1000 > NUL 2>&1]])
end

local function SetIngameMouseSensitivity()
  print("Setting ingame mouse sensitivity...")
  os.execute([[C:\Users\thomas\Desktop\mon\mouse.exe --dpi 20000 --poll-rate 500 > NUL 2>&1]])
end

function custom.gameStart(pid, name, binary)
  gcb.sendUdp("192.168.0.1", 8787, "game_start") -- Enables QoS on the router
end

function custom.gameStop(pid, name, binary)
  gcb.sendUdp("192.168.0.1", 8787, "game_stop") -- Disables QoS on the router
  SetDesktopMouseSensitivity()
end

function custom.gameForeground(pid, name, binary)
  SetIngameMouseSensitivity()
end

function custom.gameBackground(pid, name, binary)
  SetDesktopMouseSensitivity()
end
