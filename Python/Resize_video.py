import cv2
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def angle_measurement(POSE_PAIRS, p_prof, nPoints):
    teta = list()
    
    for n in range(nPoints):
        partA = np.copy(p_prof[POSE_PAIRS[n][0],:])
        partB = np.copy(p_prof[POSE_PAIRS[n][1],:])
        if((partB[0] - partA[0]) == 0):
            teta.append(90.0)
        else:
            y = partB[1] - partA[1]
            x = partB[0] - partA[0]
            angle = float(np.degrees(np.arctan(y/x)))
            if (angle > 0):
                if (x<0 and y<0):
                    angle += 180
            elif (angle < 0):
                if (x<0 and y>0):
                    angle += 180
            else:
                if (x>0):
                    angle = 0
                elif (x<0):
                    angle = 180
                else:
                    if (y>0):
                        angle = 90
                    if (y<0):
                        angle = -90
            teta.append(angle)
    print(teta)
    return np.array(teta)

def distance_measurement (POSE_PAIRS, p_prof, nPoints):
    d = list()
    for n in range(nPoints):
        partA = np.copy(p_prof[POSE_PAIRS[n][0],:])
        partB = np.copy(p_prof[POSE_PAIRS[n][1],:])
        d.append(float(np.sqrt((partA[0] - partB[0])**2+(partA[1]-partB[1])**2)))
    return np.array(d)

plt.close('all')

POSE_PAIRS = [[1,2], [2,3], [3,4], [1,5], 
             [5,6], [6,7], [1,11], [11,12],
             [12,13], [11,8], [8,9], [9,10],
             [1,0], [0,14], [14,16], [0,15], [15,17]]

nPoints = 17


p_prof = np.genfromtxt("../Photos/victor.txt",delimiter=' ',dtype=None, encoding=None)
p_stud = np.genfromtxt("../Photos/gabriel.txt",delimiter=' ',dtype=None, encoding=None)

frame = cv2.imread("../Photos/leandro.jpg")
frameCopy = np.copy(frame)

d_prof = distance_measurement(POSE_PAIRS, p_prof, nPoints)
a_prof = angle_measurement(POSE_PAIRS, p_prof, nPoints)

d_stud = distance_measurement(POSE_PAIRS, p_stud, nPoints)
a_stud = angle_measurement(POSE_PAIRS, p_stud, nPoints)

dif = 0

for n in range(nPoints):
    partA = np.copy(p_prof[POSE_PAIRS[n][0],:])
    partB = np.copy(p_prof[POSE_PAIRS[n][1],:]) + dif
    
    partB[0] = d_stud[n] * np.cos(np.deg2rad(a_prof[n])) + partA[0]
    partB[1]  = d_stud[n] * np.sin(np.deg2rad(a_prof[n])) + partA[1]

    if(n==0 or n==3 or n==6 or n==9 or n==12 or n==15):
        dif = 0
    else:
        dif = partB - p_prof[POSE_PAIRS[n][1],:]

    p_prof[POSE_PAIRS[n][1],:] = partB
    
dp_prof = distance_measurement(POSE_PAIRS, p_prof, nPoints)
ap_prof = angle_measurement(POSE_PAIRS, p_prof, nPoints)
    
init = np.copy(p_stud[1,:])
fim = np.copy(p_prof[1,:])
offset = fim - init
p_prof -= offset

dataset = pd.DataFrame({'d_stud':d_stud,'dp_prof':dp_prof,
                        'a_stud':a_stud,'ap_prof':ap_prof})