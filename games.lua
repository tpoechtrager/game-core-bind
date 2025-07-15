function getGame(name)
  return Games[name]
end

gcb.clearGameList()

for name, data in pairs(Games) do
  gcb.addGame(name, data.Binary)
end
