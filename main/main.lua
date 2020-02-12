local json = require('json')
local dump = require('dump')
local wifi = require('wifi')

print(dump.table(sys.info()))
if (not wifi.start_sta()) then
    wifi.start_ap('ESP_LUA', '')
end
print(dump.table(net.info()))
assert(sys.sntp('ntp1.aliyun.com'))
print(os.date("%Y-%m-%d %H:%M:%S"))

local base_url  = 'http://api.seniverse.com/v3/weather/now.json?key=lsawo7f7smtdljg9&language=zh-Hans&unit=c'

print(base_url)

local mqtt_connected = false
mqtt.start('mqtt://mqtt.emake.run')
local last_clock = 0
local info = {['loop'] = 0}
while (1) do
    local handle = mqtt.run()
    if (handle) then
        if (handle.event == 'MQTT_EVENT_DATA') then
            local t = json.decode(handle.data)
            print(dump.table(t))
            if (handle.topic == 'sys') then
                if (handle.data == 'stop') then -- mosquitto_pub -h mqtt.emake.run -t sys -m "\"stop\""
                    mqtt.stop()
                    break
                end
            end
        elseif (handle.event == 'MQTT_EVENT_CONNECTED') then
            mqtt_connected = true
            mqtt.sub('nc_temp', 0)
            mqtt.sub('sh_temp', 0)
            mqtt.sub('sys', 0)
        elseif (handle.event == 'MQTT_EVENT_DISCONNECTED') then
            mqtt_connected = false
        end
    end
    if (mqtt_connected and os.difftime (os.time(), last_clock) >= 30) then
        local nc_temp = web.rest('GET', base_url..'&location=nanchang')
        if (nc_temp) then
            mqtt.pub('nc_temp', nc_temp, 0)
        end
        local sh_temp = web.rest('GET', base_url..'&location=shanghai')
        if (sh_temp) then
            mqtt.pub('sh_temp', sh_temp, 0)
        end
        info.clock = os.clock()
        info.date = os.date("%Y-%m-%d %H:%M:%S")
        info.info = sys.info()
        mqtt.pub('test', json.encode(info), 1)
        last_clock = os.time()
    end
    
    info.clock = os.clock()
    info.date = os.date("%Y-%m-%d %H:%M:%S")
    info.info = sys.info()
    info.loop = info.loop + 1
    print(json.encode(info))

    if (not handle) then
        sys.yield()
    end
end
