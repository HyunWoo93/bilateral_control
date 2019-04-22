import sys
import serial
import threading
import time
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QHBoxLayout, QVBoxLayout, \
QLabel, QLineEdit, QGroupBox, QDial
from PyQt5.QtCore import Qt

def listener():
    while(True):
        #print(ex.m_port.readable())
        #print('a')
        cnt = 0
        while ex.m_port.readable():

            c = ex.m_port.read(1)
            print(c)

            # header?
            if c == b'T':
                ex.m_header = True;

                ex.m_packet[cnt] = c;
                cnt = cnt + 1

            # data
            else:
                # header is arrived
                if ex.m_header == True:
                    ex.m_packet[cnt] = c;
                    cnt = cnt + 1
                else:
                    pass


            # getting data
            if cnt == 3 :
                if ex.m_packet[0] == b'T':
                    temp = int(ex.m_packet[1])*256 + int(ex.m_packet[2])
                    ex.m_theta = (temp/10000 - PI/2)*180/PI
                    print(ex.m_theta)
                    ex.m_header == False




def speaker():
    while(True):
        while(ex.m_port.in_waiting == 0 and ex.m_response_complete and ex.bilateral_start):

            #print("insert op :")
            #op = input()

            #send(ex.m_port, op)
            send(ex.m_port, 'T' + str(ex.s_theta))
            ex.m_response_complete = False

def send(port, string):
    port.write(string.encode())
    port.write('\n'.encode())



class MyApp(QWidget):

    def __init__(self):
        super().__init__()
        

        self.m_response = ''
        self.m_response_complete = False
        self.s_response = ''
        self.s_reaponse_complete = False

        self.bilateral_start = False

        self.m_theta = 0;
        self.s_theta = 0;

        self.m_packet = [None, None, None]
        self.s_packet = [None, None, None]
        self.m_header = False;
        self.s_header = False;

        self.listener_thread = threading.Thread(target=listener)
        self.speaker_thread = threading.Thread(target=speaker)


        self.initUI()

    

    def initUI(self):

        ############## Master param ###############
        groupbox_m = QGroupBox('Master param')

        master_serial_btn = QPushButton('Master_Serial', self)
        master_serial_btn.clicked.connect(self.master_serial)

        self.m_param_port = QLineEdit(self)
        hbox1 = QHBoxLayout()
        hbox1.addWidget(QLabel('Master_port', self))
        hbox1.addWidget(self.m_param_port)

        self.m_param_k = QLineEdit(self)
        hbox2 = QHBoxLayout()
        hbox2.addWidget(QLabel('Master_K', self))
        hbox2.addWidget(self.m_param_k)

        self.m_param_c = QLineEdit(self)
        hbox3 = QHBoxLayout()
        hbox3.addWidget(QLabel('Master_C', self))
        hbox3.addWidget(self.m_param_c)


        vbox_m = QVBoxLayout()
        vbox_m.addWidget(master_serial_btn)
        vbox_m.addLayout(hbox1)
        vbox_m.addLayout(hbox2)
        vbox_m.addLayout(hbox3)

        groupbox_m.setLayout(vbox_m)

        ################ Slave_param ###############
        groupbox_s = QGroupBox('Slave param')

        slave_serial_btn = QPushButton('Slave_Serial', self)
        slave_serial_btn.clicked.connect(self.slave_serial)

        self.s_param_port = QLineEdit(self)
        hbox4 = QHBoxLayout()
        hbox4.addWidget(QLabel('Slave_port', self))
        hbox4.addWidget(self.s_param_port)

        self.s_param_k = QLineEdit(self)
        hbox5 = QHBoxLayout()
        hbox5.addWidget(QLabel('Slave_K', self))
        hbox5.addWidget(self.s_param_k)

        self.s_param_c = QLineEdit(self)
        hbox6 = QHBoxLayout()
        hbox6.addWidget(QLabel('Slave_C', self))
        hbox6.addWidget(self.s_param_c)

        vbox_s = QVBoxLayout()
        vbox_s.addWidget(slave_serial_btn)
        vbox_s.addLayout(hbox4)
        vbox_s.addLayout(hbox5)
        vbox_s.addLayout(hbox6)

        groupbox_s.setLayout(vbox_s)

        ##############################################

        hbox7 = QHBoxLayout()
        hbox7.addWidget(groupbox_m)
        hbox7.addWidget(groupbox_s)


        Master_Bias = QPushButton('Master_Bias', self)
        Master_Bias.clicked.connect(self.master_bias)
        Slave_Bias = QPushButton('Slave_Bias', self)
        Slave_Bias.clicked.connect(self.slave_bias)
        hbox8 = QHBoxLayout()
        hbox8.addWidget(Master_Bias)
        hbox8.addWidget(Slave_Bias)

        
        Bilateral_Start = QPushButton('Bilateral_Start', self)
        Bilateral_Start.clicked.connect(self.bilateral_start_fn)
        Bilateral_Stop = QPushButton('Bilateral_Stop', self)
        Bilateral_Stop.clicked.connect(self.bilateral_stop_fn)
        hbox9 = QHBoxLayout()
        hbox9.addWidget(Bilateral_Start)
        hbox9.addWidget(Bilateral_Stop)




        Master_debug = QLabel('Master_debug', self)
        Slave_debug = QLabel('Slave_debug', self)
        hbox10 = QHBoxLayout()
        hbox10.addWidget(Master_debug)
        hbox10.addWidget(Slave_debug)

        self.dial = QDial(self)
        self.dial.setRange(-40, 40)
        self.dial.valueChanged.connect(self.dial_changed)


        vbox = QVBoxLayout()
        vbox.addLayout(hbox7)
        vbox.addLayout(hbox8)
        vbox.addLayout(hbox9)
        vbox.addLayout(hbox10)
        vbox.addWidget(self.dial)

        self.setLayout(vbox)
        self.setWindowTitle('Bilateral Control (Team 2)')
        self.move(300, 300)
        self.resize(600, 800)
        self.show()

        

    def master_serial(self):
        print('master_serial')
       
        m_port = self.m_param_port.text()
        m_k = self.m_param_k.text()
        m_c = self.m_param_c.text()


        if len(m_port) == 0:
            print('you must input master port!!!!!!!!!!!!')
            sys.exit(app.exec_())

        self.m_port = serial.Serial(port='COM' + m_port, baudrate=115200)
        
        self.listener_thread.start()

        # wait for serial connection
        #time.sleep(3) # should be more than 2 sec

        # param command
        #send(self.m_port, 'C')



        if self.m_response == 'Y':
            print("parameters is set.")
        #ex.m_response = ''


        

    def slave_serial(self):
        print('slave_serial')
        self.s_port = serial.Serial(port='COM3', baudrate=115200)
        #self.listener_thread.start()
        #self.speaker_thread.start()


    def master_bias(self):
        print('master_bias')
        send(self.m_port, 'B')

    def slave_bias(self):
        print('slave_bias')

    def bilateral_start_fn(self):
        print('bilateral_start')
        self.listener_thread.start()
        self.speaker_thread.start()
        send(self.m_port, 'S')
        self.bilateral_start = True;

    def bilateral_stop_fn(self):
        print('bilateral_stop') 
        send(self.m_port, 'P')
        self.bilateral_start = False;

    def dial_changed(self, value):
        self.s_theta = value;
        print('S : ' + str(self.s_theta))
    



if __name__ == '__main__':

    app = QApplication(sys.argv)
    ex = MyApp()
    sys.exit(app.exec_())