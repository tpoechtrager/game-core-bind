-- games-gui.lua

local gameConfigWindow = nil

function showGameConfigWindow()
  if gameConfigWindow then return end

  local coreModes = {}
  for _, v in pairs(gcb.CoreBindingMode) do
    table.insert(coreModes, v)
  end

  local initWaitOptions = { "no delay", "100 ms", "250 ms", "1000 ms", "2000 ms", "5000 ms" }
  local initWaitValues = { 0, 100, 250, 1000, 2000, 5000 }

  local win = gcb.window.create {
    title = "GCB - Game Config",
    width = 800,
    height = 600,
    center = true,
    resizable = false,
    scrollable = true
  }

  gameConfigWindow = win

  local rowHeight = 28
  local spacing = 6
  local rowControls = {}

  local gameList = {}
  for name, data in pairs(Games) do
    table.insert(gameList, { name = name, data = data })
  end
  table.sort(gameList, function(a, b) return a.name < b.name end)

  local deleteButtonMap = {}

  local groupY = 10
  local groupHeight = (#gameList * (rowHeight + spacing)) + 20
  gcb.window.addGroupBox(win, 8, groupY, 760, groupHeight, "Games")

  local y = groupY + 20

  for i, entry in ipairs(gameList) do
    local name = entry.name
    local data = entry.data or {}
    local binary = data.Binary or ""
    local binding = data["Core-Binding"] or {}
    local mode = binding.Mode or gcb.CoreBindingMode.STANDARD
    local smt = binding.SMT
    if smt == nil then smt = true end
    local waitMs = data["Init-Wait"] and data["Init-Wait"].WaitMs or 0

    local nameId = gcb.window.addEditBox(win, 20, y, 150, 22, name, true)
    local binaryId = gcb.window.addEditBox(win, 180, y, 200, 22, binary)

    local modeIndex = 0
    for i, val in ipairs(coreModes) do
      if val == mode then
        modeIndex = i - 1
        break
      end
    end
    local modeId = gcb.window.addComboBox(win, 390, y, 120, 100, coreModes)
    gcb.window.setComboBoxSelectedIndex(win, modeId, modeIndex)

    local smtId = gcb.window.addCheckBox(win, 520, y, 80, 22, "SMT")
    gcb.window.setCheckBoxChecked(win, smtId, smt)

    local waitIndex = 1
    for i, v in ipairs(initWaitValues) do
      if v == waitMs then waitIndex = i break end
    end
    local waitId = gcb.window.addComboBox(win, 610, y, 100, 100, initWaitOptions)
    gcb.window.setComboBoxSelectedIndex(win, waitId, waitIndex - 1)

    local deleteId = gcb.window.addButton(win, 720, y, 40, 22, "X")
    deleteButtonMap[deleteId] = name

    rowControls[nameId] = { name = name, nameId = nameId, binaryId = binaryId, modeId = modeId, smtId = smtId, waitId = waitId }
    rowControls[binaryId] = rowControls[nameId]
    rowControls[modeId] = rowControls[nameId]
    rowControls[smtId] = rowControls[nameId]
    rowControls[waitId] = rowControls[nameId]

    y = y + rowHeight + spacing
  end

  -- Add Game section
  y = y + 20
  gcb.window.addGroupBox(win, 8, y - 4, 760, 135, "Add Game")

  gcb.window.addStatic(win, 20, y + 18, 65, 20, "Name:")
  local nameInputId = gcb.window.addEditBox(win, 90, y + 16, 200, 22)

  gcb.window.addStatic(win, 325, y + 18, 80, 20, "Binary:")
  local binaryInputId = gcb.window.addEditBox(win, 410, y + 16, 200, 22)

  gcb.window.addStatic(win, 20, y + 48, 65, 20, "Mode:")
  local addComboId = gcb.window.addComboBox(win, 90, y + 46, 200, 100, coreModes)

  local defaultModeIndex = 0
  for i, val in ipairs(coreModes) do
    if val == gcb.CoreBindingMode.X3D then
      defaultModeIndex = i - 1
      break
    end
  end
  gcb.window.setComboBoxSelectedIndex(win, addComboId, defaultModeIndex)

  gcb.window.addStatic(win, 325, y + 48, 80, 20, "Init Delay:")
  local addWaitComboId = gcb.window.addComboBox(win, 410, y + 46, 200, 100, initWaitOptions)
  gcb.window.setComboBoxSelectedIndex(win, addWaitComboId, 0)

  gcb.window.addStatic(win, 20, y + 78, 65, 20, "SMT:")
  local addCheckId = gcb.window.addCheckBox(win, 90, y + 76, 22, 22, "")
  gcb.window.setCheckBoxChecked(win, addCheckId, true)

  local addButtonId = gcb.window.addButton(win, 410, y + 74, 200, 24, "Add Game")

  gcb.window.registerCallbacks(win, {
    onEvent = function(win, id)
      if id == addButtonId then
        local name = gcb.window.getEditBoxText(win, nameInputId)
        local binary = gcb.window.getEditBoxText(win, binaryInputId)

        if not name or name == "" then
          gcb.showMessageBox("Invalid Name", "Please enter a game name.")
          return
        end

        if not binary or binary == "" then
          gcb.showMessageBox("Invalid Binary", "Please enter a binary filename.")
          return
        end

        local mode = coreModes[gcb.window.getComboBoxSelectedIndex(win, addComboId) + 1]
        local smt = gcb.window.getCheckBoxChecked(win, addCheckId)
        local wait = initWaitValues[gcb.window.getComboBoxSelectedIndex(win, addWaitComboId) + 1]

        Games[name] = {
          Binary = binary,
          ["Core-Binding"] = { Mode = mode, SMT = smt },
          ["Init-Wait"] = { WaitMs = wait }
        }

        print("Added new game: " .. name)
        gcb.window.destroy(win)
        gcb.window.unregisterCallbacks(win)
        gameConfigWindow = nil
        showGameConfigWindow()
      elseif rowControls[id] then
        local row = rowControls[id]
        local oldName = row.name
        local newName = gcb.window.getEditBoxText(win, row.nameId)
        if not newName or newName == "" then return end
        local binary = gcb.window.getEditBoxText(win, row.binaryId)
        local mode = coreModes[gcb.window.getComboBoxSelectedIndex(win, row.modeId) + 1]
        local smt = gcb.window.getCheckBoxChecked(win, row.smtId)
        local wait = initWaitValues[gcb.window.getComboBoxSelectedIndex(win, row.waitId) + 1]

        if oldName ~= newName then
          Games[oldName] = nil
          row.name = newName
        end

        Games[newName] = {
          Binary = binary,
          ["Core-Binding"] = { Mode = mode, SMT = smt },
          ["Init-Wait"] = { WaitMs = wait }
        }

        print("Updated game: " .. newName)
        print("Binary: " .. binary)
        print("Mode: " .. tostring(mode))
        print("SMT: " .. tostring(smt))
        print("Init-Wait: " .. tostring(wait) .. " ms")
      end
    end,
    onClose = function(win)
      gcb.window.destroy(win)
      gcb.window.unregisterCallbacks(win)
      gameConfigWindow = nil
    end
  })

  gcb.window.show(win)
end