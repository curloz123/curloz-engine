local sensor = {}
local Gun

function sensor.onInit()
	log.debug("Hello there, from sensor")
	Gun = entity.getEntityByName("Gun")
	log.debug(tostring(Gun))
end

function sensor.onSensorEnter(otherEntity)
	if otherEntity == Gun then
		log.info("Sensor begin from script")
	end
end
function sensor.onSensorExit(otherEntity)
	if otherEntity == Gun then
		log.info("Sensor end from script")
	end
end

log.info("Hello, from test.lua")

return sensor
