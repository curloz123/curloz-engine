local coll = {}

local gun = ecs.entity.new()
local collAudio = audio.BufferId.new()

function coll.onInit()
	gun = ecs.getEntityByName("Rose")
	coll = audio.getBufferId("assets/audio/gun_drop.wav")
end

function coll.onCollision(otherEntity, pos, speed)
	log.debug("[SCRIPT] Speed")
	log.debug(tostring(speed))
	if speed >= 25.0 then
		audio.setGain(gun, 1.0)
		audio.playPosAudio(gun, coll, pos)
	elseif speed >= 2.0 then
		audio.setGain(gun, speed / 50.0)
		audio.playPosAudio(gun, coll, pos)
	end
end

return coll
