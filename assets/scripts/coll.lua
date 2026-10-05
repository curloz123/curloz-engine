local coll = {}

local Helmet = ecs.entity.new()
local Sponza = ecs.entity.new()
local collAudio = audio.BufferId.new()
local bgAudio = audio.BufferId.new()

function coll.onInit()
	log.debug("Hello from coll.lua")
	Helmet = ecs.getEntityByName("Damaged Helmet")
	Sponza = ecs.getEntityByName("Sponza")
	coll = audio.getBufferId("assets/audio/collide_2.wav")
	bgAudio = audio.getBufferId("assets/audio/hl2.ogg")
end

local triggeredOnce = false
function coll.onCollision(otherEntity, pos)
	if not triggeredOnce then
		audio.setGain(Sponza, 3.0)
		audio.setPitch(Sponza, 0.8)
		audio.playBgAudio(Sponza, bgAudio)
		triggeredOnce = true
	end
	audio.playPosAudio(Helmet, coll, pos)
end

return coll
