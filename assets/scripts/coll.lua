local coll = {}
local Helmet
local collAudio = audio.BufferId.new()

function coll.onInit()
	log.debug("Hello from coll.lua")
	Helmet = ecs.getEntityByName("Damaged Helmet")
	coll = audio.getBufferId("assets/audio/collide_2.wav")
end

local triggeredOnce = false
function coll.onCollision(otherEntity, pos)
	audio.playPosAudio(Helmet, coll, pos)
end

return coll
