function gcb.getGame(name, caseInsensitive)
  if not caseInsensitive then
    return Games[name]
  end

  local nameLower = name:lower()
  for k, v in pairs(Games) do
    if k:lower() == nameLower then
      return v
    end
  end
  return nil
end

function gcb.getGameByBinary(binary, caseInsensitive)
  if not binary then return nil end

  if not caseInsensitive then
    for name, game in pairs(Games) do
      if game.Binary == binary then
        return name, game
      end
    end
  else
    local b = binary:lower()
    for name, game in pairs(Games) do
      if (game.Binary or ""):lower() == b then
        return name, game
      end
    end
  end

  return nil
end

gcb.clearGameList()

for name, data in pairs(Games) do
  gcb.addGame(name, data.Binary)
end

gcb.saveGames = function()
  local file = io.open("games-config.lua", "w")
  if not file then return false end

  file:write("-- This file is generated automatically by GCB. Do not edit manually.\n")
  file:write("Games = {\n")

  local function escape(str)
    return str:gsub("\\", "\\\\"):gsub("\"", "\\\"")
  end

  -- Sort game names
  local gameNames = {}
  for name in pairs(Games) do
    table.insert(gameNames, name)
  end
  table.sort(gameNames)

  for _, name in ipairs(gameNames) do
    local data = Games[name]
    local binary = escape(data.Binary or "")
    local binding = data["Core-Binding"] or {}
    local mode = binding.Mode or gcb.CoreBindingMode.X3D
    local smt = binding.SMT
    local wait = data["Init-Wait"]

    file:write(string.format("  [\"%s\"] = { Binary = \"%s\"", escape(name), binary))

    file:write(", [\"Core-Binding\"] = { Mode = ")
    if mode == gcb.CoreBindingMode.X3D then
      file:write("gcb.CoreBindingMode.X3D")
    elseif mode == gcb.CoreBindingMode.NON_X3D then
      file:write("gcb.CoreBindingMode.NON_X3D")
    else
      file:write("gcb.CoreBindingMode.STANDARD")
    end

    if smt ~= nil then
      file:write(", SMT = " .. tostring(smt))
    end
    file:write(" }")

    if wait and wait.WaitMs then
      file:write(", [\"Init-Wait\"] = { WaitMs = " .. tonumber(wait.WaitMs) .. " }")
    end

    file:write(" },\n")
  end

  file:write("}\n")
  file:close()
  return true
end
