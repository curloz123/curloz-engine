local coll = {}

local bag = ecs.entity.new()
local collAudio = audio.BufferId.new()

function coll.onInit()
	bag = ecs.getEntityByName("Briefcase")
	coll = audio.getBufferId("assets/audio/collide_2.wav")
end

function coll.onCollision(otherEntity, pos, speed)
	if speed >= 25.0 then
		audio.setGain(bag, 1.0)
		audio.playPosAudio(bag, coll, pos)
	elseif speed >= 2.0 then
		audio.setGain(bag, speed / 25.0)
		audio.playPosAudio(bag, coll, pos)
	end
end

return coll
