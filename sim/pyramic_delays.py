import json
import numpy as np
import pyroomacoustics as pra

with open('pyramic.json', 'r') as f:
    data = json.load(f)
pyramic = np.array(data['pyramic'])

t_room = 27  # degrees (temperature)
h_room = 58  # percent (humidity)
c = pra.parameters.calculate_speed_of_sound(t_room, h_room, 1000)

# a vector pointing from the source towards the array (horizontal plane)
p =  - (pyramic[7] - pyramic[0]) * np.r_[1,1,0]

delays = np.dot(pyramic, p) / c
delays = - (delays - delays.max())

print(delays)
