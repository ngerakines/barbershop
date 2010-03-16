import barbershop
import unittest
import datetime
from distutils.version import StrictVersion

class ServerCommandsTestCase(unittest.TestCase):
    
    def setUp(self):
        self.client = barbershop.Barbershop(host='localhost', port=8002)

    def test_update_and_next(self):
        # get and set can't be tested independently of each other
        self.assertEquals(self.client.next(), '-1')
        self.assertEquals(self.client.peek(), '-1')
        self.assertEquals(self.client.update('5001', 1), 'OK')
        self.assertEquals(self.client.peek(), '5001')
        self.assertEquals(self.client.next(), '5001')
        self.assertEquals(self.client.next(), '-1')
        self.assertEquals(self.client.peek(), '-1')
