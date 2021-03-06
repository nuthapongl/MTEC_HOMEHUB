import paho.mqtt.client as mqtt
import subprocess
import time
import board
import neopixel
import threading
import RPi.GPIO as GPIO
import Adafruit_ADS1x15
from numpy import interp
import json
import pygame
import re
import snowboydecoder
import sys
import signal

interrupted = False
Button_CheckNET = 27
Button_Cancel = 13
ReedSwitch_pin = 26
State_Reed =0
State_CheckNET =0
State_Cancel =0 
Red_LED = 23
Green_LED = 4
GAIN = 1
Value_ADC = 0
Bell_Send =0
start_time =-1.0
stop_time =-1.0
broker_url = "192.168.1.125"
broker_port = 1883
UUID = "16d609aa-9dde-4072-8f81-c852d5b3915f"
Connection_feedback = 1
Connection_feedback_btn =1
RED = [255,0,0]
BLUE = [0,0,255]
First_Push = 0
pixels = neopixel.NeoPixel(board.D12, 60,auto_write=False)
adc = Adafruit_ADS1x15.ADS1115()
client = mqtt.Client()
client.connect(broker_url, broker_port)
pygame.init()
pygame.mixer.init()

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(Button_CheckNET, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(Button_Cancel, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(ReedSwitch_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(Red_LED, GPIO.OUT)
GPIO.setup(Green_LED,GPIO.OUT)
Ringbell_Json ={
    "clientId" : UUID,
    "msg" : "ringbell"
}
Deniebell_Json ={
    "clientId" : UUID,
    "msg" : "cancel"
}
Callbell_Json ={
    "clientId" : UUID,
    "msg" : "callbell"
}
Connection_btn_Json ={
    "clientId" : UUID,
    "msg" : "connection_btn"
}
Connection_func_Json ={
    "clientId" : UUID,
    "msg" : "connection"
}
def main():
    time.sleep(5)
    pixels[0] = (0,0,255)
    pixels.show()
    time.sleep(1)
    pixels[0] = (0,0,0)
    pixels.show()
    Battery_Checking()
    Connection_Checking()
    thread_HW = threading.Thread(target=HW_func)
    thread_HW.start()
    thread_mqtt = threading.Thread(target=mqtt_func)
    thread_mqtt.start()
    thread_Neopixel = threading.Thread(target=Neopixel_func)
    thread_Neopixel.start()
    client.on_message = on_message
    client.subscribe("homehub", qos=0)
'''
    if len(sys.argv) == 1:
        print("Error: need to specify model name")
        print("Usage: python demo.py your.model")
        sys.exit(-1)
    model = sys.argv[1]
    signal.signal(signal.SIGINT, signal_handler)
    detector = snowboydecoder.HotwordDetector(model, sensitivity=0.5)
    print('Listening... Press Ctrl+C to exit')
    detector.start(detected_callback=voicebell,
                   interrupt_check=interrupt_callback,
                   sleep_time=0.03)
    detector.terminate()
'''   
def voicebell():
    global Callbell_Json
#     client.publish(topic="bell", payload=json.dumps(Callbell_Json), qos=0, retain=False)
    print("Voice")
def signal_handler(signal, frame):
    global interrupted
    interrupted = True

def interrupt_callback():
    global interrupted
    return interrupted    
def mqtt_func():
    global client
    client.loop_forever()
    print("Start loop")
   
def on_connect(client, userdata, flags, rc):
    print("Connected With Result Code: {}".format(rc))

def on_disconnect(client, userdata, rc):
    print("Client Got Disconnected")
   
def on_message(client, userdata, message):
    global Bell_Send ,Connection_feedback ,Connection_feedback_btn
    print("Message Recieved: "+message.payload.decode())
    sound_1= pygame.mixer.Sound("sound/ding.wav")
    data = json.loads(message.payload.decode())
#     print(pygame.mixer.get_busy())
    if(data['clientId'] == UUID):        
        if(data['msg'] == "otw"):
            print("otw")
            Bell_Send =0
            if(pygame.mixer.get_busy()) :
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/bell_otw.wav')
            elif ( not pygame.mixer.get_busy()):
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/bell_otw.wav')
                pygame.mixer.music.play(1)  
        elif(data['msg'] == "ready_btn"):
            print("ready connection btn")
            Connection_feedback_btn =1
            pixels[0] = (0,255,0)
            pixels.show()
            if(pygame.mixer.get_busy()) :
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/ready2.wav')
            elif ( not pygame.mixer.get_busy()):
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/ready2.wav')
                pygame.mixer.music.play(1)  
        elif(data['msg'] == "notready_btn"):
            print("not ready connection btn")
            pixels[0] = (0,255,0)
            pixels.show()
            Connection_feedback_btn =-1
            if(pygame.mixer.get_busy()) :
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/lost_net.wav')
            elif ( not pygame.mixer.get_busy()):
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/lost_net.wav')
                pygame.mixer.music.play(1)  
        elif(data['msg'] == "ready"):
            print("ready connection") 
            Connection_feedback =1
        elif(data['msg'] == "notready"):
            print("not ready connection") 
            Connection_feedback =-1        
        elif(data['msg'] == "success" and data['device'] == 'mo'):
            print("mo open app")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/connected_mo.wav')
            elif ( not pygame.mixer.get_busy()):
                print("Q")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/connected_mo.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == "success" and data['device'] == 'kiat'):
            print("kiat open app")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/connected_kiat.wav')
            elif ( not pygame.mixer.get_busy()):
                print("Q")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/connected_kiat.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == "success" and data['device'] == 'ja'):
            print("ja open app")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/connected_ja.wav')
            elif ( not pygame.mixer.get_busy()):
                print("Q")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/connected_ja.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == "success" and data['device'] == 'jib'):
            print("jib open app")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/connected_jib.wav')
            elif ( not pygame.mixer.get_busy()):
                print("Q")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/connected_jib.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 1 and data['device'] == 'kiat'):
            print("kiat sent text 1")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/kiat_msg1.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/kiat_msg1.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 2 and data['device'] == 'kiat'):
            print("kiat sent text 2")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/kiat_msg2.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/kiat_msg2.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 3 and data['device'] == 'kiat'):
            print("kiat sent text 3")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/kiat_msg3.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/kiat_msg3.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 1 and data['device'] == 'ja'):
            print("ja sent text 1")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/ja_msg1.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/ja_msg1.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 2 and data['device'] == 'ja'):
            print("ja sent text 2")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/ja_msg2.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/ja_msg2.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 3 and data['device'] == 'ja'):
            print("ja sent text 3")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/ja_msg3.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/ja_msg3.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 1 and data['device'] == 'jib'):
            print("jib sent text 1")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/jib_msg1.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/jib_msg1.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 2 and data['device'] == 'jib'):
            print("jib sent text 2")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/jib_msg2.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/jib_msg2.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 3 and data['device'] == 'jib'):
            print("jib sent text 3")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/jib_msg3.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/jib_msg3.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 1 and data['device'] == 'mo'):
            print("mo sent text 1")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/mo_msg1.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/mo_msg1.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 2 and data['device'] == 'mo'):
            print("mo sent text 2")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/mo_msg2.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/mo_msg2.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
        elif(data['msg'] == 3 and data['device'] == 'mo'):
            print("mo sent text 3")
            if(pygame.mixer.get_busy()) :
                print("play")
                pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/mo_msg3.wav')
                pygame.mixer.music.play(1) 
            elif ( not pygame.mixer.get_busy()):
                print("Queue")
                pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/mo_msg3.wav')
                pygame.mixer.music.play(1)
            time.sleep(0.1)
#    subprocess.call(["ifconfig" , "wlan0"])
#   if(message.payload.decode().find("New") >=0):
def Battery_Checking():
    print ("Battery Checking")
    Bat = threading.Timer(60.0, Battery_Checking)
    Bat.start()
    buffer = subprocess.check_output(["pivoyager", "status"]).decode("utf-8") 
    result = re.findall(r'[\d\.]+',buffer)
    VBat = float (result[len(result)-2])
    VRef = float (result[len(result)-1])
    if(VBat - VRef > 0.05):
        print("Battery is Good")
    elif(VBat - VRef <= 0.05):
        print("Low Battery")
        if(pygame.mixer.get_busy()) :
            pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/lowbat.wav')
            pygame.mixer.music.play(1) 
        elif ( not pygame.mixer.get_busy()):
            print("Queue")
            pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/lowbat.wav')
            pygame.mixer.music.play(1)
        time.sleep(0.1)
    print(VBat)
    print(VRef)
def Connection_Checking():
    global Connection_func_Json
    global Connection_feedback , Connection_feedback_btn
    global Green_LED , Red_LED
    print ("Connection Checking")
    client.publish(topic="bell", payload=json.dumps(Connection_func_Json), qos=0, retain=False)
    
    Con = threading.Timer(120.0, Connection_Checking)
    Con.start()
    if(Connection_feedback == 1):
        print("Ready")
        Connection_feedback=0
    elif(Connection_feedback == -1 or not ping()):
        print(" Not Ready")
        Connection_feedback = -1
        Connection_feedback_btn =-1
def Connection_Btn():
    
    global Connection_btn_Json ,Connection_feedback_btn , Connection_feedback
    client.publish(topic="bell", payload=json.dumps(Connection_btn_Json), qos=0, retain=False)
    '''
    print(ping())
    print(Connection_feedback_btn)
    if(Connection_feedback_btn == 1):
        print("Ready1")
        pixels[0] = (0,255,0)
        pixels.show()
        Connection_feedback_btn=0
    elif(Connection_feedback_btn == -1):
        print(" Not Ready1")
        pixels[0] = (0,255,0)
        pixels.show()
        Connection_feedback_btn = -1        
    '''
    if(not ping()):
        pixels[0] = (255,0,0)
        pixels.show()
        Connection_feedback_btn = -1  
def ping():
    try:
        print("ping")
        subprocess.check_output(["ping", "-c", "1", "8.8.8.8"])
        return True                      
    except subprocess.CalledProcessError:
        return False
    
def HW_func():
    global Ringbell_Json , Deniebell_Json ,Connection_feedback_btn
    global State_CheckNET , State_Reed , Bell_Send ,State_Cancel
    global start_time 
    global First_Push
    temp_reed =0
    push_time=0
    a=0
    try:
        State_Reed =0
        State_CheckNET =0
        State_Cancel =0
        State_Blink =1
        while True:
            button_state = GPIO.input(Button_CheckNET)
            reed_state = GPIO.input(ReedSwitch_pin)
            buttonc_state = GPIO.input(Button_Cancel)
#             print(button_state)
#             State_Blink =0
            if ((reed_state == 1 and State_Reed ==0) or temp_reed == 1):
                State_Reed =1
                if(First_Push == 0):
                    First_Push =1
                    temp_reed =1
                    push_time = time.time()
                    print(push_time)
                if(First_Push == 1  and (time.time() - push_time >= 6.0) and push_time != 0):
                    print('reed Pressed...' )
                    print(time.time())
#                     client.publish(topic="bell", payload=json.dumps(Ringbell_Json), qos=0, retain=False)
                    First_Push =0
                    push_time =0
                    temp_reed =0                
                    if(ping()):
                        client.publish(topic="bell", payload=json.dumps(Ringbell_Json), qos=0, retain=False)
                        Bell_Send =1
                        print("Ping Pong")
                        start_time = time.time()
                    else:
                        print("error connection")
                        if(pygame.mixer.get_busy()) :
                            pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/lost_net.wav') # ??????????????????????????????????????????????????????????????????
                        elif ( not pygame.mixer.get_busy()):
                            pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/lost_net.wav')
                            pygame.mixer.music.play(1)
                            
            elif (reed_state == 0 and State_Reed ==1):                
                State_Reed =0
                
            if(Bell_Send == 1  and time.time() - start_time >= 5.0):
                print(time.time() - start_time)
                print("DONE")
                Bell_Send =0
                if(pygame.mixer.get_busy()) :
                    pygame.mixer.music.queue('/home/pi/BellProject/Test_Snow/sound/lost_net.wav') # ??????????????????????????????????????????????????????????????????
                elif ( not pygame.mixer.get_busy()):
                    pygame.mixer.music.load('/home/pi/BellProject/Test_Snow/sound/lost_net.wav')
                    pygame.mixer.music.play(1)
            #print (Connection_feedback_btn )         
            if (button_state == 1 and State_CheckNET ==0):
                State_CheckNET =1
                a+=1
                print('Button Pressed...' )
                print(a)                
                Connection_Btn()
                time.sleep(0.3)
#                 Connection_feedback_btn =0
                #Connection_Btn()
            
            elif (button_state == 0 and State_CheckNET ==1):            
                State_CheckNET =0
                time.sleep(0.2)
                
            if (buttonc_state == 1 and State_Cancel ==0):
                State_Cancel =1
                print('Button C Pressed...')
                client.publish(topic="bell", payload=json.dumps(Deniebell_Json), qos=0, retain=False)
                time.sleep(0.2)
            elif (buttonc_state == 0 and State_Cancel ==1):            
                State_Cancel =0
                time.sleep(0.2)
#             print(pygame.mixer.get_busy())
    except :
        GPIO.cleanup()
        
def Neopixel_func ():
    global GAIN , Value_ADC ,State_Blink ,Connection_feedback_btn
    global Connection_feedback
#     State_Blink =1
    while True:
        Value_ADC = adc.read_adc(0, gain=GAIN)
        Bright = int(interp(Value_ADC, [100,20000], [0,255]))
#          print( int ((100/255)*Bright ))
        for i in range(1,59):
            pixels[i] = (int ((230/255)*Bright), int ((190/255)*Bright), 0)
        pixels.show()
        '''
        if(Connection_feedback_btn == 1 or Connection_feedback ==1):
            pixels[0] = (0,255,0)
            pixels.show()
        if(Connection_feedback_btn == -1 or Connection_feedback_btn == 0 ):
            pixels[0] = (255,0,0)
            pixels.show()
            print("B")
        elif(Connection_feedback == -1 or Connection_feedback == 0 ):
            print("A")
            pixels[0] = (255,0,50)
            pixels.show()
        '''
#         time.sleep(0.1)

def pixel_blink(Color):
    print("blink")
    for i in range (5):
        pixels.fill((255,0,0))
        pixels.show()
        time.sleep(0.3)
        pixels.fill((0,0,0))
        pixels.show()
        time.sleep(0.3)
if __name__ == "__main__":
   try:
      main()
   except KeyboardInterrupt:
      # do nothing here
      pixels.fill((0, 0, 0))
      pixels.show()
      GPIO.cleanup()
      
