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
ser.write(b'AA')

def readlineCR(port):
    rv = ""
    start=False
    
    while True:
        # print(start)
        ch = port.read()
        # print(len(ch))
        # print(type(ch))
        # print(ch)
        try: 
            if ch.decode('utf-8')=='{':
                start=True
        except:
            pass
        if start:
            try:
                rv+=ch.decode('utf-8')
                
            except:
                print('Fail to decode')
                pass
            # print('rv: {0}'.format(rv))
            try:
                if rv.find('}\r\nEOL',0)!=-1:
                    start=False
                    # print('rv: {0}'.format(rv))
                    return rv[:-3]
            except:
                print('Fail to find EOL')
                pass

def validate_json(text):
    try:
        if text.__contains__('tim') & text.__contains__('rpm') &text.__contains__('vol') &text.__contains__('cur'):
            return True
    except:
        return False
with open ('Data.csv','w', newline='') as csvfile:
    print(time.strftime('%X %x'))
    fieldnames=['vol','cur','rpm','tim']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames,dialect='excel-tab')
    writer.writeheader()
    while ser.isOpen():
        # print(timestamp)
        # print(ser.isOpen())
        
        # if ser.readline() is not 'EOL':
            # message+=str(ser.readline())
            # print(message)
        message = readlineCR(ser)
        # print('Message: {0}'.format(message))
        # print('count open: {0} count close: {1}'.format(message.count('{'),message.count('}')))
        # print(message)
        if message.count('{')==1 & message.count('}')==1 :
            try:
                text= json.loads(message)
                if validate_json(text):
                    timestamp=int(text['tim'])
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
        
        if timestamp>5*60*1000000:# 1 minutes
            print(time.strftime('%X %x'))
            ser.write(b'S')
            sys.exit()

        
        
        # diff=(timestamp-timestamp_last)
        # diff_diff=abs(diff-diff_last)
        # freq=1/(diff/1000000)
        # print(round(freq,2))

        # print(diff)
        # timestamp_last=timestamp
        # diff_last=diff

