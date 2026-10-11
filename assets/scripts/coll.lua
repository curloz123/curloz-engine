local coll = {}

local Helmet = ecs.entity.new()
local Sponza = ecs.entity.new()
local collAudio = audio.BufferId.new()
local bgAudio = audio.BufferId.new()

function coll.onInit()
	log.debug("Hello from coll.lua")
	Helmet = ecs.getEntityByName("Damaged Helmet")
	Sponza = ecs.getEntityByName("Sponza")
	coll = audio.getBufferId("assets/audio/collide.wav")
	bgAudio = audio.getBufferId("assets/audio/hl2.ogg")
end

local triggeredOnce = false
function coll.onCollision(otherEntity, pos, speed)
	if not triggeredOnce then
		audio.setGain(Sponza, 1.0)
		audio.setPitch(Sponza, 1.0)
		-- audio.playBgAudio(Sponza, bgAudio)
		triggeredOnce = true
	end
	if speed >= 10.0 then
		audio.setGain(Helmet, 1.0)
		audio.playPosAudio(Helmet, coll, pos)
	elseif speed >= 2 then
		audio.setGain(Helmet, speed / 10.0)
		audio.playPosAudio(Helmet, coll, pos)
	end
end

return coll
