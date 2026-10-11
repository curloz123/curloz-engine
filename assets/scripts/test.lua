local sensor= {}

local collSound = audio.BufferId.new()
local helmet

function sensor.onInit()
	helmet = ecs.getEntityByName("Damaged Helmet")
	collSound = audio.getBufferId("assets/audio/collide.wav")
end

function sensor.onSensorEnter(otherEntity)
	if otherEntity == Gun then
		log.info("Gun Sensor begin")
	end
end
function sensor.onSensorExit(otherEntity)
	if otherEntity == Gun then
		log.info("Gun Sensor end")
	end
end

return sensor
