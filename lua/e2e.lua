local tym = require('tym')

print('start')

tym.set_timeout(function()
  print('done')
  tym.quit()
end, 1000)
