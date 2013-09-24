local field = {}
field.tile = {}
field.IMAGE = "data/tilesets/crops.png"

function field:load( args )
	-- load the image into memory
	self.img = game.newImage()
	self.img:load( field.IMAGE )
	self.canUpdate = true
end

function field:update( ms, x, y )
	if not self.canUpdate then 
		return nil
	end
	
	for i = 1, data.field.width * data.field.height do
		local tile = data.field.tile[i]
		if tile.water or tile.till > 0 then
			-- load the image if not loaded
			if self.tile[i] == nil then
				local img = game.newImage()
				img:load( field.IMAGE )
				
				-- position the image properly
				local x = (i-1) % data.field.width * 32
				local y = math.floor( (i-1) / data.field.width ) * 32
				img:position( x, y )
				
				self:addImage( img )
				self.tile[i] = img
			end
			
			-- watered and implicitly tilled
			if tile.water then
				self.tile[i]:subrect( 32, 0, 32, 32 )
			else -- only tilled, but not watered
				self.tile[i]:subrect( 0, 0, 32, 32 )
			end
		elseif self.tile[i] ~= nil and ( not tile.water and tile.till == 0  )then
			self:removeImage( self.tile[i] )
			self.tile[i] = nil
		end
	end
end

function field:interact( x, y )
	x, y = math.floor( x / 32 ), math.floor( y / 32 )
	local i = y * data.field.width + x + 1
	
	local tile = data.field.tile[i]
	
	if tile.till > 0 then
		tile.water = true
	else
		tile.till = 1
	end
	
	self.canUpdate = true
end

function field:hasCollision( x, y )
	return false
end

return field
