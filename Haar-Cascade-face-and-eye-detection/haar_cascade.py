import numpy as np
import cv2
import datetime

face_cascade=cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
eye_cascade=cv2.CascadeClassifier('haarcascade_eye.xml')

cap=cv2.VideoCapture(0)
fps_start_time = datetime.datetime.now()
total_frames = 0
# cap = cv2.VideoCapture('rtsp://192.168.1.32')
while True:
    ret,frame=cap.read()
    total_frames += 1
    
    gray=cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)
    faces=face_cascade.detectMultiScale(gray,1.1,10)
 

    for(x,y,w,h) in faces:
        cv2.rectangle(frame,(x,y),(x+w,y+h),(255,0,0),2)
        roi_gray=gray[y:y+h,x:x+h]
        roi_color=frame[y:y+h,x:x+h]
        eyes=eye_cascade.detectMultiScale(roi_gray)
        for(ex,ey,ew,eh) in eyes:
            cv2.rectangle(roi_color,(ex,ey),(ex+ew,ey+eh),(0,255,0),2)

   
    fps_end_time = datetime.datetime.now()
    td = (fps_end_time - fps_start_time).seconds
    if td == 0:
        fps = 0.0
    else:
        fps = total_frames / (fps_end_time - fps_start_time).seconds
    #========
    FPS = 'FPS : ' + str(np.round(fps))
    sz = str(int(cap.get(3))) + ' x ' + str(int(cap.get(4)))
    cv2.putText(frame, FPS, (0, 650), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.putText(frame, sz, (0, 700), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.imshow('face',frame)
    #========
    if cv2.waitKey(1) & 0xff==ord('v'):
            break       
    

   

cap.release()
cv2.destroyAllWindows()
