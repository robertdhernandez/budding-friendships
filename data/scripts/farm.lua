local field = { width = 20, height = 20 }

-- initialize field tiles
field.tile = {}
for i = 1, field.width * field.height do
	local tile = {}
	tile.till = 0
	tile.water = false
	
	field.tile[i] = tile
end

function field.getTile( x, y )
	return field.tile[ y * field.width + x + 1]
end

-- save the field to data table
data.field = field
