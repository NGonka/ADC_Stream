'''
Python 3.4 code !!!
'''
import serial,json,time,csv,sys



ser = serial.Serial('COM4',115200)

message=''

time.sleep(2.0)
timestamp_last=0
timestamp=0
diff_last=0
ser.write(b'DA')
time.sleep(1.0)
start=False
print(time.strftime('%X %x'))
def readlineCR(port):
    rv = ""
    
    
    while True:
        # print(start)
        ch = port.read()
        # print(len(ch))
        # print(type(ch))
        # print(ch)
        try:
            rv+=ch.decode('utf-8')
            
        except:
            print('Fail to decode')
            return False
        # print('rv: {0}'.format(rv))
        try:
            if rv.find('EOH\r\n',0)!=-1:
                start=True
                # print('rv: {0}'.format(rv))
                rv=''
                
                return start
        except:
            print('Fail to find EOH')
            return False

with open ('Data.csv','w', newline='') as csvfile:
    fieldnames=['voltage','current','power','rpm','timestamp']
    writer = csv.writer(csvfile, delimiter=',')
    writer.writerow(fieldnames)
    start =readlineCR(ser)
    # print(start)
    # print (ser.readline())
    while ser.isOpen():        
        if start==True:
            message = ser.readline().decode()
            # print(str(message))
            # print(str(message).split(','))
            data = str(message).split(',')
            # print(data)
            data2=[int(x) for x in data]
            # print(data2)
            timestamp = data2[-1]
            # print(timestamp)
            if timestamp>0:
                writer.writerow(data2)
                # pass
        '''
        # print('Message: {0}'.format(message))
        # print('count open: {0} count close: {1}'.format(message.count('{'),message.count('}')))
        if message.count('{')==1 & message.count('}')==1 :
            try:
                text= json.loads(message)
                if validate_json(text):
                    timestamp=int(text['timestamp'])
                    # print(text['timestamp'])
                    if timestamp>0:             #2*60*1000000:
                        writer.writerow(text)
                else:
                    message=''
                    # timestamp=0
                    ser.flushOutput()
            except ValueError as err:
                print('JSON Error')
                print (err)
                print('Faulty Message: {0}'.format(message))
                message=''
                # timestamp=0
                ser.flushOutput()
                pass
        else:
            message=''
            # timestamp=0
            ser.flushOutput()
 '''       
        if timestamp>4*60*1000000:
            ser.write(b'S')
            print(time.strftime('%X %x'))
            sys.exit()

        
'''        
        # diff=(timestamp-timestamp_last)
        # diff_diff=abs(diff-diff_last)
        # freq=1/(diff/1000000)
        # print(round(freq,2))

        # print(diff)
        # timestamp_last=timestamp
        # diff_last=diff
    
'''