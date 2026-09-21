local sensor = {}
local Gun

function sensor.onInit()
	Gun = entity.getEntityByName("Gun")
	log.debug(tostring(Gun))
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
