local sensor = {}
local Gun

function sensor.onInit()
	log.debug("Hello there, from sensor script 2")
	Gun = entity.getEntityByName("Gun")
	log.debug(tostring(Gun))
end

function sensor.onSensorEnter(otherEntity)
	if otherEntity == Gun then
		log.info("Sensor begin from script 2")
	end
end
function sensor.onSensorExit(otherEntity)
	if otherEntity == Gun then
		log.info("Sensor end from script 2")
	end
end

log.info("Hello, from test2.lua")

return sensor
