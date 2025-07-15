gcb.window._handlers = {}

function gcb.window.registerCallbacks(win, callbacks)
  if not win then return end
  gcb.window._handlers[win] = {
    onEvent = callbacks.onEvent,
    onClose = callbacks.onClose
  }
end

function gcb.window.unregisterCallbacks(win)
  gcb.window._handlers[win] = nil
end

function gcb.onWindowEvent(win, id)
  local handler = gcb.window._handlers[win]
  if handler and type(handler.onEvent) == "function" then
    handler.onEvent(win, id)
  end
end

function gcb.onWindowClose(win)
  local handler = gcb.window._handlers[win]
  if handler and type(handler.onClose) == "function" then
    handler.onClose(win)
  else
    gcb.window.destroy(win)
  end
  gcb.window._handlers[win] = nil
end
