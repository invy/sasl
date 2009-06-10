size = { 160, 160 }

-- initialize component property table

defineProperty("gyro", globalPropertyf("sim/cockpit2/gauges/indicators/heading_electric_deg_mag_pilot"))
defineProperty("fail", globalPropertyf("sim/operation/failures/rel_ss_dgy"))
defineProperty("gauge_power", globalPropertyi("sim/custom/xap/gauge_power_avail"))
defineProperty("course", 0)

-- background image
defineProperty("background", loadImage("uk_2.png", 0, 0, 150, 150))

-- needle image
defineProperty("triangle", loadImage("uk_2.png", 62, 187, 24, 21))
defineProperty("scale", loadImage("uk_2.png", 151, 151, 105, 105))
defineProperty("plane_needle", loadImage("uk_2.png", 170, 6, 67, 130))


local angle = get(gyro)

function update()
    local power = get(gauge_power)
    local v = get(gyro) + get(course)
 
    -- check if indicator is working  
    if get(fail) < 6 and get(gauge_power) == 1 then
        -- calculate smooth move of adf needle
        local delta = v - angle
        if delta > 180 then delta = delta - 360 end
        if delta < -180 then delta = delta + 360 end
       
        angle = angle + 0.5 * delta 

        -- calculate circle of needle's move
        if angle > 180 then angle = angle - 360 end
        if angle < -180 then angle = angle + 360 end
    end   

    return true
end                                                              


-- gurocompas consists of several components
components = {

    -- background image
    texture { 
        position = { 5, 5, 150, 150 },
        image = background,
    },

    -- round scale
    needle {
        position = { 18, 22, 120, 120 },
        image = get(scale),
        angle = function() 
             return get(gyro) + get(course)
        end,
    },    
    
    -- triangle (marker) image
    texture { 
        position = { 70, 113, 15, 15 },
        image = triangle,
    },
   


    -- gyro-course needle (plane)
    needle {
        position = { 21, 25, 115, 115 },
        image = get(plane_needle),
        angle = function() 
             return angle
        end,
    },  

    -- heading rotary
    rotary {
        position = { 0, 0, 40, 40 };
        image = loadImage("rotary.png");
        value = course;
        adjuster = function(value)
            if 360 >= value then
                value = value - 360
            end
            if 0 > value then
                value = value + 360
            end
            return value
        end
    },
   
}

