import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import serial
import time
import serial.tools.list_ports as ser2
from std_msgs.msg import String, Int16MultiArray

class SenderNode(Node):
    def __init__(self):
        super().__init__('SenderNode')
        

#self.subscription = self.create_subscription(Int16MultiArray, 'gimbal_control', self.control_callback, 10)
        self.subscription = self.create_subscription(String, 'Ugv_Link', self.publish_message, 10)


#ser2.ListPortInfo()
        #ports = ser2.grep("10c4:ea60")#"Silicon Labs CP210x UART Bridge")
        #dynamic port finder
        ports = ser2.grep("0403:6015")# Future Technology Devices International, Ltd Bridge(I2C/SPI/UART/FIFO) serial radio

        #sends angles to gimbal
        Gimbal_angles = Int16MultiArray()#painful to write in c++ so its here
        Gimbal_angles.data = [50,0]#fix angles 135 0?
        self.publisher = self.create_publisher(Int16MultiArray, 'gimbal_control', 10)
        time.sleep(3)
        
        self.publisher.publish(Gimbal_angles)
        device = "none"

        for p in ports:
            device = p[0]
        self.get_logger().info(f'2 Way Radio Port port {device}')



        # config serial port
        self.serial_port = serial.Serial(device, baudrate=57600, timeout=100) # note: change /dev/tyUSB0 to my port
        
        # # ROS publisher (for debugging)
        # self.publisher_ = self.create_publisher(String, 'send_data', 10)
        
        # boolean indicating whether aruco marker location has been found
        self.loc_found = False
        msg1 = String()
        msg1.data = 'Gimbal Angle set'
        self.publish_message(msg1)
        # timer to update location constantly
        #####self.timer = self.create_timer(1.0, self.update_loc_found)
    # end def __init__

    def publish_message(self, msg):
        #msg = String()
        #msg.data = 'Coordinates: 123.456, 789.101'
        
        # send msg over USB radio
        self.serial_port.write((msg.data + '\n').encode('utf-8'))
        
        # log sent msg
        self.get_logger().info(f'Sent over radio: {msg.data}')
    # end def publish_message

    def update_loc_found(self):
        self.loc_found = 'True' == (input('Has the aruco marker location been found? (True/False) '))
        if self.loc_found:
            self.publish_message()
            self.timer.cancel()
        # end if statement
    # end def update_loc_found
# end class SenderNode

def main(args=None):
    rclpy.init(args=args)
    node = SenderNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
# end def main

if __name__ == '__main__':
    main()
# end main program area
