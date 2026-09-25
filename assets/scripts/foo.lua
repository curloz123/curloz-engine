local sensor = {}
local Helmet
local coll = audio.BufferId.new()

function sensor.onInit()
	log.debug("Hello there, from foo.lua")
	Helmet = ecs.getEntityByName("Damaged Helmet")
	log.debug(tostring(Helmet))
	coll = audio.getBufferId("assets/audio/hl2.ogg")
	log.debug(tostring(coll:getId()))
end

function sensor.onSensorEnter(otherEntity)
	if otherEntity == Helmet then
		log.info("Helmet Sensor begin")
		audio.playAudio(Helmet, coll)	
	end
end
function sensor.onSensorExit(otherEntity)
	if otherEntity == Helmet then
		log.info("Helmet Sensor end")
	end
end

return sensor
