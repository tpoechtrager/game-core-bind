local ID_EXIT = 100
local ID_CONFIG_DISABLE_DESKTOP_EFFECTS = 101
local ID_CONFIG_SET_CPU_AFFINITY = 102
local ID_CONFIG_DISABLE_NON_PRIMARY_DISPLAYS = 103
local ID_CONFIG_ENSURE_RUNNING_AS_ADMIN = 104
local ID_OPEN_GAMES_FILE = 200
local ID_VERSION_INFO = 300

-- Cleanup existing tray if needed
gcb.tray.destroy()

-- Initialize tray with version in tooltip
local tooltip = string.format("Game Core Bind - Version %d (%s)", gcb.version.Build, gcb.version.GitRev)
gcb.tray.init(tooltip)

-- Version info as disabled menu entry
local versionText = string.format("Version %d (%s)", gcb.version.Build, gcb.version.GitRev)
gcb.tray.addMenuItem(versionText, ID_VERSION_INFO)

-- Open games.lua
gcb.tray.addMenuItem("Open games.lua", ID_OPEN_GAMES_FILE)

-- Create config submenu
local configMenu = gcb.tray.createSubMenu()

gcb.tray.addMenuItemToSubMenu(configMenu, "Disable Desktop Effects (When Ingame)", ID_CONFIG_DISABLE_DESKTOP_EFFECTS)
gcb.tray.addMenuItemToSubMenu(configMenu, "Set CPU Affinity", ID_CONFIG_SET_CPU_AFFINITY)
gcb.tray.addMenuItemToSubMenu(configMenu, "Disable Non-Primary Displays (When Ingame)", ID_CONFIG_DISABLE_NON_PRIMARY_DISPLAYS)
gcb.tray.addMenuItemToSubMenu(configMenu, "Ensure Running As Admin", ID_CONFIG_ENSURE_RUNNING_AS_ADMIN)

-- Attach submenu
gcb.tray.addSubMenu("Config", configMenu)

-- Exit menu item
gcb.tray.addMenuItem("Exit", ID_EXIT)

-- Set initial check states
gcb.tray.setMenuChecked(ID_CONFIG_DISABLE_DESKTOP_EFFECTS, Config.DisableDesktopEffects)
gcb.tray.setMenuChecked(ID_CONFIG_SET_CPU_AFFINITY, Config.SetCpuAffinity)
gcb.tray.setMenuChecked(ID_CONFIG_DISABLE_NON_PRIMARY_DISPLAYS, Config.DisableNonPrimaryDisplays)
gcb.tray.setMenuChecked(ID_CONFIG_ENSURE_RUNNING_AS_ADMIN, Config.EnsureRunningAsAdmin)

-- Event callback for menu clicks
function gcb.onTrayEvent(id)
  if id == ID_EXIT then
    gcb.shutdown()
  elseif id == ID_CONFIG_DISABLE_DESKTOP_EFFECTS then
    local state = not gcb.tray.isMenuChecked(id)
    gcb.tray.setMenuChecked(id, state)
    Config.DisableDesktopEffects = state
    gcb.saveConfig()
  elseif id == ID_CONFIG_DISABLE_NON_PRIMARY_DISPLAYS then
    local state = not gcb.tray.isMenuChecked(id)
    gcb.tray.setMenuChecked(id, state)
    Config.DisableNonPrimaryDisplays = state
    gcb.saveConfig()
  elseif id == ID_CONFIG_ENSURE_RUNNING_AS_ADMIN then
    local state = not gcb.tray.isMenuChecked(id)
    gcb.tray.setMenuChecked(id, state)
    Config.EnsureRunningAsAdmin = state
    gcb.saveConfig()
    if state and not gcb.isRunningAsAdmin() then
      gcb.restartAsAdmin()
    end
  elseif id == ID_OPEN_GAMES_FILE then
    gcb.runDetached("notepad.exe", "games.lua")
  elseif id == ID_VERSION_INFO then
    gcb.showMessageBox(
      "GCB Version",
      string.format(
        "You are using GCB Version %d (%s).\n\n%s",
        gcb.version.Build,
        gcb.version.GitRev,
        "https://github.com/tpoechtrager/game-core-bind"
      )
    )
  end
end
