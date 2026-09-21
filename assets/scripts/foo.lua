local sensor = {}
local Helmet

function sensor.onInit()
	Helmet = entity.getEntityByName("Damaged Helmet")
end

function sensor.onSensorEnter(otherEntity)
	if otherEntity == Helmet then
		log.info("Helmet Sensor begin")
	end
end
function sensor.onSensorExit(otherEntity)
	if otherEntity == Helmet then
		log.info("Helmet Sensor end")
	end
end

return sensor
